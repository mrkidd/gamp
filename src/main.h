/*
 *  Gnome Audio Player main init and loop
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
GtkWidget *label_time;
GAPPlayer *gamp_gp;
GladeXML *xml;
GtkTreeView *playlist_treeview;

void update_currently_playing (char *uri, gboolean get_info);
void update_time (int time);
void cb_gap_player_eos (GAPPlayer *gp);
void cb_gap_player_tick (GAPPlayer *gp);
void error_dialog (const char *error_text);

#endif
