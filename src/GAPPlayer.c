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
#include "gap-metadata.h"

static void gap_player_class_init (GAPPlayerClass *klass);
static void gap_player_init (GAPPlayer *gp);
static void gap_player_finalize (GObject *object);

void gap_player_construct (GAPPlayer *gp, gboolean iradio);
static gboolean gap_idle_handler (gpointer data);
static gboolean gap_tick_timeout_cb (GAPPlayer *gp);

static void eos_signal_cb (GstElement *gstelement, GAPPlayer *gp);
static gboolean eos_signal_idle (GAPPlayer *gp);

struct GAPPlayerPrivate
{
	GstElement *pipeline;
	GstElement *filesrc;
	GstElement *decoder;
	GstElement *audiosink;
	
	char *vfsuri;
	long duration;

	GTimer *timer;
	guint tick_timeout;
	long timer_add;
		
	gboolean playing;
	gboolean mute;
};

typedef enum {
	EOS,
	TICK,
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
	gap_player_signals[TICK] = g_signal_new ("tick", G_OBJECT_CLASS_TYPE (object_class), G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GAPPlayerClass, tick), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void gap_player_init (GAPPlayer *gp)
{
	gint ms_period = 1000 / 5;
	
	gp->_priv = g_new0 (GAPPlayerPrivate, 1);
	
	gp->_priv->tick_timeout = g_timeout_add (ms_period, (GSourceFunc) gap_tick_timeout_cb, gp);
}

static void gap_player_finalize (GObject *object)
{
	GAPPlayer *gp;
	
	gp = GAP_PLAYER (object);

	g_source_remove (gp->_priv->tick_timeout);
	
	gap_close (gp);

	if (gp->_priv->timer)
		g_timer_destroy (gp->_priv->timer);
			
	g_free (gp->_priv);
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GAPPlayer *gap_player_new (void)
{
	GAPPlayer *gp;
	
	gp = GAP_PLAYER (g_object_new (GAP_PLAYER_TYPE, NULL));
	
	return gp;
}

static gboolean
gap_tick_timeout_cb (GAPPlayer *gp)
{
	if (gp->_priv->playing == FALSE)
		return TRUE;
		
/*	g_signal_emit (G_OBJECT (gp), gap_player_signals[TICK], 0);*/
	
	return TRUE;
}

static gboolean gap_idle_handler (gpointer data)
{
	GAPPlayer *gp = GAP_PLAYER (data);
	GstElement *gst_pipeline = GST_ELEMENT (gp->_priv->pipeline);
	
	return gst_bin_iterate (GST_BIN (gst_pipeline));
}

void gap_player_construct (GAPPlayer *gp, gboolean iradio)
{
	g_return_if_fail (IS_GAP_PLAYER (gp));
	
	gp->_priv->pipeline = gst_pipeline_new ("pipeline");

	gp->_priv->filesrc = gst_element_factory_make ("gnomevfssrc", "src");
	if (gp->_priv->filesrc == NULL)
	{
		error_dialog ("Could not load the GnomeVFS plugin, check your Gstreamer installation");
		g_object_unref (GST_OBJECT (gp->_priv->pipeline));
		return;
	}

	g_printf ("Construct: iradio is %d\n", iradio);	
	if (iradio)
		gp->_priv->decoder = gst_element_factory_make ("mad", "autoplugger");
	else
		gp->_priv->decoder = gst_element_factory_make ("spider", "autoplugger");
	if (gp->_priv->decoder == NULL)
	{
		error_dialog ("Could not load the Spider plugin, check your Gstreamer installation");
		gst_object_unref (GST_OBJECT (gp->_priv->filesrc));
		g_object_unref (GST_OBJECT (gp->_priv->pipeline));
		return;
	}
	
	gp->_priv->audiosink = gst_gconf_get_default_audio_sink ();
	if (gp->_priv->audiosink == NULL)
	{
		error_dialog ("Could not load the default output plugin, check your Gstreamer installation");
		gst_object_unref (GST_OBJECT (gp->_priv->filesrc));
		gst_object_unref (GST_OBJECT (gp->_priv->decoder));
		g_object_unref (GST_OBJECT (gp->_priv->pipeline));
		return;
	}
	
	gst_bin_add_many (GST_BIN (gp->_priv->pipeline), gp->_priv->filesrc, gp->_priv->decoder, gp->_priv->audiosink, NULL);
	gst_element_link_many (gp->_priv->filesrc, gp->_priv->decoder, gp->_priv->audiosink, NULL);
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_READY);
	
	if (gp->_priv->timer)
		g_timer_destroy (gp->_priv->timer);
	gp->_priv->timer = g_timer_new ();
	g_timer_stop (gp->_priv->timer);
	g_timer_reset (gp->_priv->timer);
	gp->_priv->timer_add = 0;
}

void gap_open (GAPPlayer *gp, char *vfsuri)
{
	gboolean iradio_mode = FALSE;
	
	g_return_if_fail (IS_GAP_PLAYER (gp));
	
	if (vfsuri && g_str_has_prefix (vfsuri, "http://"))
		iradio_mode = TRUE;
		
	if (gp->_priv->pipeline)
	{
		gst_element_set_state (gp->_priv->pipeline, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (gp->_priv->pipeline));
		gp->_priv->pipeline = NULL;
	}

	g_free (gp->_priv->vfsuri);
	gp->_priv->vfsuri = NULL;

	if (vfsuri == NULL)
	{
		gp->_priv->playing = FALSE;
		return;
	}

	g_printf ("gp vfsuri: %s, iradio: %d\n", vfsuri, iradio_mode);
	gap_player_construct (gp, iradio_mode);

	g_object_set (G_OBJECT (gp->_priv->filesrc), "iradio-mode", iradio_mode, NULL);
	g_object_set (G_OBJECT (gp->_priv->filesrc), "location", vfsuri, NULL);
	gp->_priv->vfsuri = g_strdup (vfsuri);
	
	gap_get_metadata (gp, NULL, NULL, NULL);
}

void gap_play (GAPPlayer *gp)
{
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_PLAYING);
	gp->_priv->playing = TRUE;
	
	g_timer_start (gp->_priv->timer);
	
	g_idle_add ((GSourceFunc) gap_idle_handler, gp);
	g_signal_connect (G_OBJECT (gp->_priv->audiosink), "eos", G_CALLBACK (eos_signal_cb), gp);
}

void gap_pause (GAPPlayer *gp)
{
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_PAUSED);
	gp->_priv->timer_add += floor (g_timer_elapsed (gp->_priv->timer, NULL) + 0.5);
	g_timer_stop (gp->_priv->timer);
	g_timer_reset (gp->_priv->timer);
}

void gap_stop (GAPPlayer *gp)
{
	g_return_if_fail (IS_GAP_PLAYER (gp));
	
	gp->_priv->playing = FALSE;
	
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_READY);
	g_timer_stop (gp->_priv->timer);
	g_timer_reset (gp->_priv->timer);
	gp->_priv->timer_add = 0;
	g_idle_remove_by_data (gp->_priv->pipeline);
}

void gap_close (GAPPlayer *gp)
{
	g_return_if_fail (IS_GAP_PLAYER (gp));

	gap_stop (gp);

	g_free (gp->_priv->vfsuri);
	gp->_priv->vfsuri = NULL;

	if (gp->_priv->pipeline == NULL)
		return;
		
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (gp->_priv->pipeline));
	gp->_priv->pipeline = NULL;
	
	g_object_set (G_OBJECT (gp->_priv->filesrc), "location", NULL, NULL);
	g_object_unref (G_OBJECT (gp->_priv->filesrc));
}

long gap_get_duration (GAPPlayer *gp)
{
	return gp->_priv->duration;
}

long gap_get_time (GAPPlayer *gp)
{
	GstClock *gst_clock;
	long time;
	
	g_return_val_if_fail (IS_GAP_PLAYER (gp), -1);

	if (gp->_priv->pipeline != NULL)
	{	
		time = (long) floor (g_timer_elapsed (gp->_priv->timer, NULL) + 0.5) + gp->_priv->timer_add;
	}
	else
		time = -1;
	
	return time;
}

void gap_set_time (GAPPlayer *gp, long time)
{
	GstEvent *gst_event;
	
	g_return_if_fail (IS_GAP_PLAYER (gp));
	g_return_if_fail (time >= 0);
	
	g_return_if_fail (gp->_priv->pipeline != NULL);
	
	gst_element_set_state (gp->_priv->pipeline, GST_STATE_PAUSED);
	gst_event = gst_event_new_seek (GST_FORMAT_TIME | GST_SEEK_METHOD_SET | GST_SEEK_FLAG_FLUSH,
									time * GST_SECOND);
	gst_element_send_event (gp->_priv->audiosink, gst_event);
	
	if (gp->_priv->playing == TRUE)
		gst_element_set_state (gp->_priv->pipeline, GST_STATE_PLAYING);
		
	g_timer_reset (gp->_priv->timer);
	gp->_priv->timer_add = time;
}

void gap_get_metadata (GAPPlayer *gp, char **artist, char **title, long *duration)
{
	GAPMetaData *gapmd;

	gapmd = g_new0 (GAPMetaData, 1);
	gap_metadata_load (gapmd, gp->_priv->vfsuri);
	if (artist != NULL)
		*artist = g_strdup (gapmd->artist);
	if (title != NULL)
		*title = g_strdup (gapmd->title);
	if (duration != NULL)
		*duration = gapmd->duration;
	gp->_priv->duration = gapmd->duration;
	g_printf ("get_metadata duration: %d; gp duration: %d\n", gapmd->duration, gp->_priv->duration);

	g_free (gapmd);
}

void gap_get_metadata_uri (const char *uri, char **artist, char **title, long *duration)
{
	GAPMetaData *gapmd;
	
	gapmd = g_new0 (GAPMetaData, 1);
	gap_metadata_load (gapmd, uri);
	*artist = g_strdup (gapmd->artist);
	*title = g_strdup (gapmd->title);
	*duration = gapmd->duration;
	g_free (gapmd);
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
