/*
 *  Callbacks and misc functions related to the GUI
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
#include <glade/glade.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "main.h"
#include "playlist.h"
#include "GAPPlayer.h"

void on_open1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_open_location1_activate (GtkButton *button, gpointer user_data);
void on_quit1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data);
gboolean on_window1_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_window_playlist_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data);

void on_button_previous_clicked (GtkButton *button, gpointer user_data);
void on_button_play_clicked (GtkButton *button, gpointer user_data);
void on_button_pause_clicked (GtkButton *button, gpointer user_data);
void on_button_stop_clicked (GtkButton *button, gpointer user_data);
void on_button_next_clicked (GtkButton *button, gpointer user_data);
void on_button_eject_clicked (GtkButton *button, gpointer user_data);
void on_togglebutton_shuffle_toggled (GtkToggleButton *togglebutton, gpointer user_data);
void on_togglebutton_repeat_toggled (GtkToggleButton *togglebutton, gpointer user_data);
void on_togglebutton_equalizer_toggled (GtkToggleButton *togglebutton, gpointer user_data);
void on_togglebutton_playlist_toggled (GtkToggleButton *togglebutton, gpointer user_data);

void on_treeview_playlist_row_activated (GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer user_data);
void on_button_add_clicked (GtkButton *button, gpointer user_data);
void on_button_remove_clicked (GtkButton *button, gpointer user_data);
void on_button_load_clicked (GtkButton *button, gpointer user_data);
void on_button_save_clicked (GtkButton *button, gpointer user_data);
void on_button_clear_clicked (GtkButton *button, gpointer user_data);

void on_button_open_clicked (GtkButton *button, gpointer user_data);
void on_button_cancel_clicked (GtkButton *button, gpointer user_data);

void cb_file_open (GtkWidget *widget, gpointer user_data);
void cb_file_add (GtkWidget *widget, gpointer user_data);
void cb_playlist_load (GtkWidget *widget, gpointer user_data);

void on_open1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *file_selector;
	
	file_selector = gtk_file_selection_new ("Please select an Audio file");
	g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button), 
		"clicked", G_CALLBACK (cb_file_open), (gpointer) file_selector);
	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
		"clicked", G_CALLBACK (gtk_widget_destroy), (gpointer) file_selector);
	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
		"clicked", G_CALLBACK (gtk_widget_destroy), (gpointer) file_selector);
	
	gtk_widget_show (file_selector);
}

void on_open_location1_activate (GtkButton *button, gpointer user_data)
{
	GtkWidget *dialog_location, *entry_location, *content;
	GladeXML *dialog_xml;
	char *uri_entry;
	GnomeVFSURI *vfsuri;

	dialog_location = gtk_dialog_new_with_buttons ("Open Location",
							 GTK_WINDOW (main_window), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL,
							 GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	g_return_if_fail (dialog_location != NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog_location), GTK_RESPONSE_OK);
	gtk_window_set_resizable (GTK_WINDOW (dialog_location), FALSE);

	dialog_xml = glade_xml_new (PACKAGE_SOURCE_DIR"/data/open-location.glade", "dialog_open_location_content", NULL);
	if (dialog_xml == NULL)
		dialog_xml = glade_xml_new (PACKAGE_DATA_DIR"/open-location.glade", "dialog_open_location_content", NULL);
	content = glade_xml_get_widget (dialog_xml, "dialog_open_location_content");
	g_return_if_fail (content != NULL);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog_location)->vbox), content, FALSE, FALSE, 0);	
	
	entry_location = glade_xml_get_widget (dialog_xml, "entry_location");
	g_return_if_fail (entry_location != NULL);

	switch (gtk_dialog_run (GTK_DIALOG (dialog_location)))
	{
		case GTK_RESPONSE_OK:
		{
			uri_entry = gtk_editable_get_chars (GTK_EDITABLE (entry_location), 0, -1);
			if (uri_entry != NULL)
			{
				vfsuri = gnome_vfs_uri_new (uri_entry);
				if (vfsuri != NULL)
				{
					gap_open (gamp_gp, uri_entry);
					gap_play (gamp_gp);
					update_currently_playing (uri_entry, TRUE);
					playlist_add_item ("", "0:00", uri_entry);
					gnome_vfs_uri_unref (vfsuri);
				}
			}
			break;
		}
		default:
			break;
	}
	gtk_widget_destroy (dialog_location);
}

void on_quit1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_main_quit ();
}

void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
}

gboolean on_window1_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_main_quit ();
	
	return FALSE;
}

gboolean on_window_playlist_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GtkWidget *tbutton;
	
	tbutton = glade_xml_get_widget (xml, "togglebutton_playlist");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tbutton), FALSE);
	
	return TRUE;
}

void on_button_previous_clicked (GtkButton *button, gpointer user_data)
{
	playlist_previous ();
}

void on_button_play_clicked (GtkButton *button, gpointer user_data)
{
	gap_play (gamp_gp);
}

void on_button_pause_clicked (GtkButton *button, gpointer user_data)
{
	gap_pause (gamp_gp);
}

void on_button_stop_clicked (GtkButton *button, gpointer user_data)
{
	gap_stop (gamp_gp);
}

void on_button_next_clicked (GtkButton *button, gpointer user_data)
{
	playlist_next ();
}

void on_button_eject_clicked (GtkButton *button, gpointer user_data)
{
}

void on_togglebutton_shuffle_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
}

void on_togglebutton_repeat_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
}

void on_button_volume_clicked (GtkButton *button, gpointer user_data)
{
}

void on_togglebutton_playlist_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
	if (GTK_WIDGET_VISIBLE (playlist_window))
		gtk_widget_hide (playlist_window);
	else
		gtk_widget_show (playlist_window);
}

void on_treeview_playlist_row_activated (GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer user_data)
{
	GtkTreeIter iter;
	gboolean ret_val;
	GtkTreeModel *tree_model;
	char *selected_uri;
	char *selected_title;
	
	tree_model = gtk_tree_view_get_model (treeview);
	ret_val = gtk_tree_model_get_iter (tree_model, &iter, arg1);
	
	gtk_tree_model_get (tree_model, &iter, 2, &selected_uri, -1);
	gtk_tree_model_get (tree_model, &iter, 0, &selected_title, -1);
	
	gap_open (gamp_gp, selected_uri);
	update_currently_playing (selected_title, FALSE);
	gap_play (gamp_gp);
	
	g_free (selected_uri);
}

void on_button_add_clicked (GtkButton *button, gpointer user_data)
{
	GtkWidget *file_selector;
	
	file_selector = gtk_file_selection_new ("Please select an Audio file");
	g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button), 
		"clicked", G_CALLBACK (cb_file_add), (gpointer) file_selector);
	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
		"clicked", G_CALLBACK (gtk_widget_destroy), (gpointer) file_selector);
	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
		"clicked", G_CALLBACK (gtk_widget_destroy), (gpointer) file_selector);
	
	gtk_widget_show (file_selector);
}

void on_button_remove_clicked (GtkButton *button, gpointer user_data)
{
	playlist_remove_selected (playlist_treeview);
}

void on_button_load_clicked (GtkButton *button, gpointer user_data)
{	
	GtkWidget *file_selector;
	
	file_selector = gtk_file_selection_new ("Please select an Audio file");
	g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button), 
		"clicked", G_CALLBACK (cb_playlist_load), (gpointer) file_selector);
	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
		"clicked", G_CALLBACK (gtk_widget_destroy), (gpointer) file_selector);
	g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
		"clicked", G_CALLBACK (gtk_widget_destroy), (gpointer) file_selector);
	
	gtk_widget_show (file_selector);
}

void on_button_save_clicked (GtkButton *button, gpointer user_data)
{
}

void on_button_clear_clicked (GtkButton *button, gpointer user_data)
{
	playlist_clear ();
}

void cb_file_open (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector;
	const char *mimetype;
	char *selected_uri;
	
	file_selector = (GtkWidget *) user_data;
	selected_uri = gnome_vfs_get_uri_from_local_path (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));
	mimetype = gnome_vfs_get_mime_type (selected_uri);

	if (g_ascii_strcasecmp (mimetype, "audio/x-scpls") == 0)
		playlist_add_pls (selected_uri);
	else
	{
		gap_open (gamp_gp, selected_uri);
		gap_get_metadata (gamp_gp);
		update_currently_playing (selected_uri, TRUE);
		playlist_add_item ("", "0:00", selected_uri);
		gap_play (gamp_gp);
	}
}

void cb_file_add (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector;
	const char *mimetype;
	char *selected_uri;
	
	file_selector = (GtkWidget *) user_data;
	selected_uri = gnome_vfs_get_uri_from_local_path (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));
	mimetype = gnome_vfs_get_mime_type (selected_uri);

	if (g_ascii_strcasecmp (mimetype, "audio/x-scpls") == 0)
		playlist_add_pls (selected_uri);
	else
	{
		playlist_add_item ("", "0:00", selected_uri);
	}
}

void cb_playlist_load (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector;
	const char *mimetype;
	char *selected_uri;
	
	file_selector = (GtkWidget *) user_data;
	selected_uri = gnome_vfs_get_uri_from_local_path (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));
	mimetype = gnome_vfs_get_mime_type (selected_uri);
	if (g_ascii_strcasecmp (mimetype, "audio/x-scpls") == 0)
		playlist_add_pls (selected_uri);
}

void update_currently_playing (char *title, gboolean get_info)
{
	char *currently_playing;
	GnomeVFSFileInfo file_info;

	if (get_info == TRUE)
	{
		gnome_vfs_get_file_info (title, &file_info, GNOME_VFS_FILE_INFO_DEFAULT);
		currently_playing = g_strdup_printf ("<b><big>%s</big></b>", file_info.name);
	}
	else
		currently_playing = g_strdup_printf ("<b><big>%s</big></b>", title);		

	g_markup_escape_text (currently_playing, strlen (currently_playing));
	gtk_label_set_markup (GTK_LABEL (label_current_song), currently_playing);

	g_free (currently_playing);	
}

void cb_gap_player_eos (GAPPlayer *gp)
{
	playlist_next ();
}
