/*
 * File: GAPPlayer.h
 * Created by: Mason Kidd <mrkidd@mrkidd.com>
 * Created on: Fri Oct 10 14:52:43 2003
 * 
 * Gnome Audio Player object definition
 *
 */

#ifndef _GAPPLAYER_H_
#define _GAPPLAYER_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gnome.h>

G_BEGIN_DECLS

#define GAP_PLAYER_TYPE				(gap_player_get_type ())
#define GAP_PLAYER(obj)				(GTK_CHECK_CAST ((obj), GAP_PLAYER_TYPE, GAPPlayer))
#define GAP_PLAYER_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), GAP_PLAYER_TYPE, GAPPlayerClass))
#define IS_GAP_PLAYER(obj)			(GTK_CHECK_TYPE ((obj), GAP_PLAYER_TYPE))
#define IS_GAP_PLAYER_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GAP_PLAYER_TYPE))
#define GAP_PLAYER_GET_CLASS(obj)	(GTK_CHECK_GET_CLASS ((obj), GAP_PLAYER_TYPE, GAPPlayerClass))

typedef struct GAPPlayerPrivate GAPPlayerPrivate;
	
typedef struct
{
	GObject parent;
	
	GAPPlayerPrivate *_priv;
} GAPPlayer;

typedef struct
{
	GObjectClass parent_class;

	void (*eos) (GAPPlayer *gp);
} GAPPlayerClass;

GType gap_player_get_type (void);
GAPPlayer *gap_player_new (void);

void gap_open (GAPPlayer *gp, char *vfsuri);
void gap_play (GAPPlayer *gp);
void gap_pause (GAPPlayer *gp);
void gap_stop (GAPPlayer *gp);
void gap_close (GAPPlayer *gp);

void gap_get_metadata (GAPPlayer *gp);

long gap_get_time (GAPPlayer *gp);
void gap_set_time (GAPPlayer *gp, long time);

G_END_DECLS

#endif	/*_GAPPLAYER_H_*/
