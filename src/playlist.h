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
