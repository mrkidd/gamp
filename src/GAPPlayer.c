/*
 *  GAPPlayer object implementation
 *
 *  Copyright (C) 2003 Mason Kidd <mrkidd@mrkidd.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#include <gst/gst.h>
#include <gst/gconf/gconf.h>
#include <gst/media-info/media-info.h>

#include "GAPPlayer.h"

static void gap_player_class_init (GAPPlayerClass *klass);
static void gap_player_init (GAPPlayer *gp);
static void gap_player_finalize (GObject *object);

static void gap_player_construct (GAPPlayer *gp);
static gboolean gap_idle_handler (gpointer data);

static void eos_signal_cb (GstElement *gstelement, GAPPlayer *gp);
static gboolean eos_signal_idle (GAPPlayer *gp);

struct GAPPlayerPrivate
{
	GstElement *pipeline;
	GstElement *filesrc;
	GstElement *decoder;
	GstElement *audiosink;
	
	char *vfsuri;
	
	gboolean playing;
	gboolean mute;
};

typedef enum {
	EOS,
	LAST_SIGNAL
} GAPPlayerSignalType;

static guint gap_player_signals[LAST_SIGNAL] = {0};

static GObjectClass *parent_class = NULL;

GType gap_player_get_type (void)
{
	static GType type = 0;
	
	if (type == 0)
	{
		static const GTypeInfo our_info =
		{
			sizeof (GAPPlayerClass),
			NULL,
			NULL,
			(GClassInitFunc) gap_player_class_init,
			NULL,
			NULL,
			sizeof (GAPPlayer),
			0,
			(GInstanceInitFunc) gap_player_init,
		};
		
		type = g_type_register_static (G_TYPE_OBJECT, "GAPPlayer", &our_info, 0);
	}
	
	return type;
}

static void gap_player_class_init (GAPPlayerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	parent_class = g_type_class_peek_parent (klass);
	
	object_class->finalize = gap_player_finalize;
	
	gap_player_signals[EOS] = g_signal_new ("eos", G_OBJECT_CLASS_TYPE (object_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GAPPlayerClass, eos), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void gap_player_init (GAPPlayer *gp)
{
	gp->_priv = g_new0 (GAPPlayerPrivate, 1);
}

static void gap_player_finalize (GObject *object)
{
	GAPPlayer *gp;
	
	gp = GAP_PLAYER (object);
	
	gap_close (gp);
	
	g_free (gp->_priv);
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GAPPlayer *gap_player_new (void)
{
	GAPPlayer *gp;
	
	gp = GAP_PLAYER (g_object_new (GAP_PLAYER_TYPE, NULL));
	
	gap_player_construct (gp);
	
	return gp;
}

static gboolean gap_idle_handler (gpointer data)
{
	GstElement *gst_pipeline = GST_ELEMENT (data);

	return gst_bin_iterate (GST_BIN (gst_pipeline));
}

static void gap_player_construct (GAPPlayer *gp)
{
	gp->_priv->playing = FALSE;
	
	gp->_priv->pipeline = gst_pipeline_new ("pipeline");
	gp->_priv->filesrc = gst_element_factory_make ("gnomevfssrc", "src");
	if (gp->_priv->filesrc == NULL)
	{
		g_print ("Could not create GnomeVFSSrc object\n");
		g_object_unref (GST_OBJECT (gp->_priv->pipeline));
	}
	g_object_set (G_OBJECT (gp->_priv->filesrc), "location", NULL, NULL);
	
	gp->_priv->decoder = gst_element_factory_make ("spider", "autoplugger");
	// check if NULL
	
	gp->_priv->audiosink = gst_gconf_get_default_audio_sink ();
	// check if NULL
	
	gst_bin_add_many (GST_BIN (gp->_priv->pipeline), gp->_priv->filesrc, gp->_priv->decoder, gp->_priv->audiosink, NULL);
	gst_element_link_many (gp->_priv->filesrc, gp->_priv->decoder, gp->_priv->audiosink, NULL);
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_READY);
}

void gap_open (GAPPlayer *gp, char *vfsuri)
{
	g_return_if_fail (IS_GAP_PLAYER (gp));
	
	g_free (gp->_priv->vfsuri);
	gp->_priv->vfsuri = NULL;
	if (vfsuri == NULL)
	{
		g_object_set (G_OBJECT (gp->_priv->filesrc), "location", NULL, NULL);
		gp->_priv->playing = FALSE;
		return;
	}

	gst_element_set_state (gp->_priv->pipeline, GST_STATE_READY);
	gp->_priv->vfsuri = g_strdup (vfsuri);
	g_object_ref (G_OBJECT (gp->_priv->filesrc));
	g_object_set (G_OBJECT (gp->_priv->filesrc), "location", vfsuri, NULL);
}

void gap_play (GAPPlayer *gp)
{
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_PLAYING);
	gp->_priv->playing = TRUE;
	
	g_idle_add ((GSourceFunc) gap_idle_handler, gp->_priv->pipeline);
	g_signal_connect (G_OBJECT (gp->_priv->audiosink), "eos", G_CALLBACK (eos_signal_cb), gp);
}

void gap_pause (GAPPlayer *gp)
{
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_PAUSED);
}

void gap_stop (GAPPlayer *gp)
{
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_READY);
	g_idle_remove_by_data (gp->_priv->pipeline);
	gp->_priv->playing = FALSE;
}

void gap_close (GAPPlayer *gp)
{
	g_return_if_fail (IS_GAP_PLAYER (gp));

	gap_stop (gp);
	
	g_object_set (G_OBJECT (gp->_priv->filesrc), "location", NULL, NULL);
	g_object_unref (G_OBJECT (gp->_priv->filesrc));
}

long gap_get_time (GAPPlayer *gp)
{
	GstClock *gst_clock;
	long time;
	
	g_return_val_if_fail (IS_GAP_PLAYER (gp), -1);
	
	gst_clock = gst_bin_get_clock (GST_BIN (gp->_priv->pipeline));
	time = (long) gst_clock_get_time (gst_clock);
	
	return time;
}

void gap_set_time (GAPPlayer *gp, long time)
{
	GstEvent *gst_event;
	
	g_return_if_fail (IS_GAP_PLAYER (gp));
	g_return_if_fail (time >= 0);
	
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_PAUSED);
	gst_event = gst_event_new_seek (GST_SEEK_METHOD_SET | GST_SEEK_FLAG_FLUSH, time);
	gst_element_send_event (gp->_priv->audiosink, gst_event);
	
	if (gp->_priv->playing == TRUE)
		gst_element_set_state (gp->_priv->pipeline, GST_STATE_PLAYING);
}

void gap_get_metadata (GAPPlayer *gp)
{
/*	const GList *pads_list;
	GstCaps *caps;

	artist = g_strdup_printf ("Korn");
	title = g_strdup_printf ("Blah");
	g_print ("%s\n", gst_object_get_path_string (GST_OBJECT (gp->_priv->decoder)));

	GstCaps *caps_metadata;
	GstProps *props_metadata;
	const GList *props_list;
	GstElement *element;

	element = gst_bin_get_by_name (GST_BIN (gp->_priv->decoder), "src_0");
	g_object_get (G_OBJECT (element), "metadata", &caps_metadata, NULL);
	props_metadata = gst_caps_get_props (caps_metadata);
	props_list = props_metadata->properties;
	
	while (props_list)
	{
		g_print ("Prop ");
		props_list = g_list_next (props_list);
	}
	GList *elements;
  
  elements = gst_bin_get_list (GST_BIN (gp->_priv->decoder));

  while (elements) {
    GstElement *element = GST_ELEMENT (elements->data);

    g_print ("element in bin: %s\n", GST_OBJECT_NAME (GST_OBJECT (element)));

    elements = g_list_next (elements);
  }*/
}

static void eos_signal_cb (GstElement *gstelement, GAPPlayer *gp)
{
	g_idle_add ((GSourceFunc) eos_signal_idle, gp);
}

static gboolean eos_signal_idle (GAPPlayer *gp)
{
	g_signal_emit (G_OBJECT (gp), gap_player_signals[EOS], 0);
	
	return 0;
}
