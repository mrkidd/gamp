/*
 *  Playlist implementation
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
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>

#include "main.h"
#include "playlist.h"

enum {
	COLUMN_TITLE,
	COLUMN_TIME,
	COLUMN_URI,
	N_COLUMNS
};

GtkListStore *liststore_playlist;

void playlist_create_list (GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	liststore_playlist = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_tree_view_set_model (treeview, GTK_TREE_MODEL (liststore_playlist));

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Title", renderer, "text", COLUMN_TITLE, NULL);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (treeview, column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Time", renderer, "text", COLUMN_TIME, NULL);
	gtk_tree_view_append_column (treeview, column);
}

void playlist_add_item (char *title, char *time, char *uri)
{
	GtkTreeIter iter_add;
	GnomeVFSFileInfo file_info;
	GnomeVFSResult vfs_result;
	
	g_print ("Adding: %s\n", uri);
	gtk_list_store_append (liststore_playlist, &iter_add);
	gtk_list_store_set (liststore_playlist, &iter_add,
				COLUMN_TITLE, title,
				COLUMN_TIME, time,
				COLUMN_URI, uri,
				-1);
}

void playlist_add_pls (char *uri)
{
	GnomeVFSResult vfs_result;
	int file_size, i, number_of_entries, ret_val;
	char *file_contents, **file_lines, *delimiter, **file_line;
	
	vfs_result = gnome_vfs_read_entire_file (uri, &file_size, &file_contents);
	/* check return */

	delimiter = "\n";
	file_lines = g_strsplit (file_contents, delimiter, 0);
	g_free (file_contents);

	ret_val = g_ascii_strcasecmp (file_lines[0], "[playlist]");
	if (ret_val != 0)
		return;

	file_line = g_strsplit (file_lines[1], "=", 2);
	ret_val = g_ascii_strcasecmp (file_line[0], "numberofentries");
	if (ret_val != 0)
		return;
	
	number_of_entries = g_strtod (file_line[1], NULL);
	if (number_of_entries <= 0)
		return;

	for (i = 2; i < (number_of_entries * 2); i++)
	{
		file_line = g_strsplit (file_lines[i], "=", 2);
		if (g_ascii_strncasecmp (file_line[0], "file", 4) == 0)
		{
			char *t_artist = NULL, *t_title = NULL, *t_format = NULL;
			long t_duration;
			char *s_duration;
                                 
			gap_get_metadata_uri (file_line[1], &t_artist, &t_title, &t_duration);
			t_format = g_strdup_printf ("%s - %s", t_artist, t_title);
			s_duration = g_strdup_printf ("%d:%02d", t_duration / 60, t_duration %60);
			playlist_add_item (t_format, s_duration, file_line[1]);
			g_free (t_format);
			g_free (t_artist);
			g_free (t_title);
		}
	}
	
	g_strfreev (file_lines);
	g_strfreev (file_line);
}

void playlist_remove_selected (GtkTreeView *treeview)
{
	GtkTreeSelection *current_selection;
	GtkTreeIter iter;
	
	current_selection = gtk_tree_view_get_selection (treeview);
	gtk_tree_selection_get_selected (current_selection, NULL, &iter);
	gtk_list_store_remove (liststore_playlist, &iter);
}

void playlist_play_selected (void)
{
	GtkTreeSelection *current_selection;
	GtkTreeIter iter;
	char *selected_uri;
	char *selected_title;
	GtkTreeModel *tree_model;
	
	tree_model = gtk_tree_view_get_model (playlist_treeview);
	current_selection = gtk_tree_view_get_selection (playlist_treeview);
	gtk_tree_selection_get_selected (current_selection, NULL, &iter);
	
	gtk_tree_model_get (tree_model, &iter, 0, &selected_title, -1);
	gtk_tree_model_get (tree_model, &iter, 2, &selected_uri, -1);
	
	gap_open (gamp_gp, selected_uri);
	update_currently_playing (selected_title, FALSE);
	gap_play (gamp_gp);
}

void playlist_play_and_sel_first (void)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	char *selected_uri;
	char *selected_title;
	GtkTreeModel *tree_model;
	
	tree_model = gtk_tree_view_get_model (playlist_treeview);
	path = gtk_tree_path_new_first ();
	gtk_tree_view_set_cursor (playlist_treeview, path, NULL, FALSE);
	gtk_tree_model_get_iter_first (tree_model, &iter);
	
	gtk_tree_model_get (tree_model, &iter, 0, &selected_title, -1);
	gtk_tree_model_get (tree_model, &iter, 2, &selected_uri, -1);
	gap_open (gamp_gp, selected_uri);
	update_currently_playing (selected_title, FALSE);
	gap_play (gamp_gp);
}

void playlist_clear (void)
{
	gtk_list_store_clear (liststore_playlist);
}

void playlist_next (void)
{
	GtkTreeSelection *current_selection;
	GtkTreeIter iter;
	gboolean ret_val;
	GtkTreeModel *tree_model;
	char *selected_uri, *selected_title;

	tree_model = gtk_tree_view_get_model (playlist_treeview);
	current_selection = gtk_tree_view_get_selection (playlist_treeview);
	gtk_tree_selection_get_selected (current_selection, NULL, &iter);
	ret_val = gtk_tree_model_iter_next (tree_model, &iter);
	if (ret_val == FALSE)
	{
		update_currently_playing ("Not Playing", FALSE);
	}
	else
	{
		gtk_tree_selection_select_iter (current_selection, &iter);

		gtk_tree_model_get (tree_model, &iter, 2, &selected_uri, -1);
		gtk_tree_model_get (tree_model, &iter, 0, &selected_title, -1);
	
		gap_open (gamp_gp, selected_uri);
		update_currently_playing (selected_title, FALSE);
		gap_play (gamp_gp);
	
		g_free (selected_uri);
		g_free (selected_title);
	}
}

void playlist_previous (void)
{
	GtkTreeSelection *current_selection;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *tree_model;
	char *selected_uri, *selected_title;

	tree_model = gtk_tree_view_get_model (playlist_treeview);
	current_selection = gtk_tree_view_get_selection (playlist_treeview);
	gtk_tree_selection_get_selected (current_selection, NULL, &iter);

	path = gtk_tree_model_get_path (tree_model, &iter);
	if (!gtk_tree_path_prev (path))
	{
		gtk_tree_path_free (path);
		return;
	}
	gtk_tree_model_get_iter (tree_model, &iter, path);
	gtk_tree_selection_select_iter (current_selection, &iter);

	gtk_tree_model_get (tree_model, &iter, 2, &selected_uri, -1);
	gtk_tree_model_get (tree_model, &iter, 0, &selected_title, -1);
	gap_open (gamp_gp, selected_uri);
	update_currently_playing (selected_title, FALSE);
	gap_play (gamp_gp);
	
	g_free (selected_uri);
}
