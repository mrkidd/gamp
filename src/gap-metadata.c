#include <config.h>
#include <gst/gst.h>
#include <gst/gsttag.h>
#include <string.h>

#include "gap-metadata.h"

static void cb_gst_error (GstElement *element, GstElement *source, GError *error, 
				gchar *debug, GAPMetaData *md);
static void cb_gst_found_tag (GObject *pipeline, GstElement *source, GstTagList *tags, 
				GAPMetaData *md);
static void cb_gst_have_type (GstElement *typefind, guint probability, GstCaps *caps, 
				GAPMetaData *md);
static void cb_gst_handoff (GstElement *fakesink, GstBuffer *buf, GstPad *pad, GAPMetaData *md);
static void cb_gst_eos (GstElement *element, GAPMetaData *md);

void gap_metadata_load (GAPMetaData *md, const char *uri)
{
        GstElement *pipeline = NULL;
        GstElement *gnomevfssrc = NULL;
        GstElement *typefind = NULL;
        GstElement *spider = NULL;
	GstElement *fakesink = NULL;
        GstCaps *filtercaps = NULL;
        const char *plugin_name = NULL;

	g_return_if_fail (uri != NULL);

	g_print ("Loading metadata for: %s\n", uri);
	md->uri = g_strdup (uri);
	md->duration = -1;

        /* The main tagfinding pipeline looks like this:
         * gnomevfssrc ! typefind ! spider ! application/x-gst-tags ! fakesink
         */
        pipeline = gst_pipeline_new ("pipeline");
	g_signal_connect (pipeline, "error", G_CALLBACK (cb_gst_error), md);
	g_signal_connect (pipeline, "found-tag", G_CALLBACK (cb_gst_found_tag), md);

	gnomevfssrc = gst_element_factory_make ("gnomevfssrc", "gnomevfssrc");
	/* TODO: check error */
	gst_bin_add (GST_BIN (pipeline), gnomevfssrc);
	g_object_set (G_OBJECT (gnomevfssrc), "location", uri, NULL);
	
	typefind  = gst_element_factory_make ("typefind", "typefind");
	/* TODO: check error */
	g_signal_connect (typefind, "have_type", G_CALLBACK (cb_gst_have_type), md);
	gst_bin_add (GST_BIN (pipeline), typefind);

	spider = gst_element_factory_make ("spider", "spider");
	/* TODO: check error */
	gst_bin_add (GST_BIN (pipeline), spider);

	fakesink = gst_element_factory_make ("fakesink", "fakesink");
	/* TODO: check error */
	gst_bin_add (GST_BIN (pipeline), fakesink);
	g_object_set (G_OBJECT (fakesink), "signal-handoffs", TRUE, NULL);
	g_signal_connect (fakesink, "handoff", G_CALLBACK (cb_gst_handoff), md);
	g_signal_connect (fakesink, "eos", G_CALLBACK (cb_gst_eos), md);

	gst_element_link_many (gnomevfssrc, typefind, spider, NULL);

	filtercaps = gst_caps_new_simple ("audio/x-raw-int", NULL);
	gst_element_link_filtered (spider, fakesink, filtercaps);
	gst_caps_free (filtercaps);

	gst_element_set_state (pipeline, GST_STATE_PLAYING);
	while ((gst_bin_iterate (GST_BIN (pipeline)))
		&& (md->error == NULL)
		&& (!md->handoff)
		&& (!md->eos))
		;

	if (md->handoff)
	{
		if (md->duration == -1)
		{
			GstFormat format = GST_FORMAT_TIME;
			gint64 length;
	
			if (gst_element_query (fakesink, GST_QUERY_TOTAL, &format, &length))
			{
				GValue *newval = g_new0 (GValue, 1);
				g_print ("Duration query succeeded\n");
				g_value_init (newval, G_TYPE_LONG);
				g_value_set_long (newval, (long) (length / (1 * 1000 * 1000 * 1000)));
				md->duration = g_value_get_long (newval);
				g_printf ("Duration: %d\n", md->duration);
			}
			else
				g_print ("Duration query failed\n");
		}
	}
	else if (md->eos)
		g_print ("Received eos without handoff\n");

	gst_element_set_state (pipeline, GST_STATE_NULL);

	if (pipeline != NULL)
		gst_object_unref (GST_OBJECT (pipeline));
}


static void 
cb_gst_error (GstElement *element, GstElement *source, GError *error, gchar *debug, GAPMetaData *md)
{
	g_printf ("Caught error: %s\n", error->message);
}

static void 
cb_gst_found_tag (GObject *pipeline, GstElement *source, GstTagList *tags, GAPMetaData *md)
{
	gboolean retval;
	glong duration;
	char *temp = NULL;

	retval = gst_tag_list_get_long (tags, GST_TAG_DURATION, &duration);
	if (retval)
	{
		md->duration = duration/(1000*1000*1000);
		g_printf ("Duration found: %d\n", md->duration);
	}

	retval = gst_tag_list_get_string (tags, GST_TAG_TITLE, &temp);
	if (retval)
	{
		g_printf ("Title found: %s\n", temp);
		md->title = g_strdup (temp);
	}
	temp = NULL;
	retval = gst_tag_list_get_string (tags, GST_TAG_ARTIST, &temp);
	if (retval)
	{
		g_printf ("Artist found: %s\n", temp);
		md->artist = g_strdup (temp);
	}
	
}

static void 
cb_gst_have_type (GstElement *typefind, guint probability, GstCaps *caps, GAPMetaData *md)
{
	if (gst_caps_get_size (caps) > 0)
	{
		md->type = g_strdup (gst_structure_get_name (gst_caps_get_structure (caps, 0)));
		g_printf ("Found type: %s\n", md->type);
	}
}

static void 
cb_gst_handoff (GstElement *fakesink, GstBuffer *buf, GstPad *pad, GAPMetaData *md)
{
	if (md->handoff)
		g_print ("Caught recursive handoff\n");
	else if (md->eos)
		g_print ("Caught handoff after eos\n");
	else
	{
		g_print ("In fakesink handoff\n");
		md->handoff = TRUE;
	}
}

static void 
cb_gst_eos (GstElement *element, GAPMetaData *md)
{
	g_print ("Received EOS\n");
	if (md->eos)
	{
		g_print ("Reentered EOS\n");
		return;
	}

	gst_element_set_state (md->sink, GST_STATE_NULL);
	md->eos = TRUE;
}

