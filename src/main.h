#ifndef _GAMP_MAIN_H_
#define _GAMP_MAIN_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <glade/glade.h>

#include "GAPPlayer.h"

GtkWidget *playlist_window;
GtkWidget *main_window;
GtkWidget *label_current_song;
GAPPlayer *gamp_gp;
GladeXML *xml;
GtkTreeView *playlist_treeview;

void update_currently_playing (char *uri, gboolean get_info);
void cb_gap_player_eos (GAPPlayer *gp);

#endif
