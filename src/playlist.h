/*
 *  Playlist definitions
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
#ifndef _GAMP_PLAYLIST_H_
#define _GAMP_PLAYLIST_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

void playlist_create_list (GtkTreeView *treeview);
void playlist_add_item (char *title, char *time, char *uri);
void playlist_add_pls (char *uri);
void playlist_remove_selected (GtkTreeView *treeview);
void playlist_play_selected (void);
void playlist_play_and_sel_first (void);
void playlist_clear (void);
void playlist_next (void);
void playlist_previous (void);

#endif
