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
#include <libgnomeui/gnome-about.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "main.h"
#include "playlist.h"
#include "GAPPlayer.h"

void on_open1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_open_location1_activate (GtkButton *button, gpointer user_data);
void on_quit1_activate (GtkMenuItem *menuitem, gpointer user_data);

void on_open2_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_save1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_clear1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_add_file1_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_remove_file1_activate (GtkMenuItem *menuitem, gpointer user_data);

void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data);

gboolean on_window_main_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_expander_playlist_activate (GtkExpander *expander, gpointer user_data);

void on_button_previous_clicked (GtkButton *button, gpointer user_data);
void on_button_play_clicked (GtkButton *button, gpointer user_data);
void on_button_pause_clicked (GtkButton *button, gpointer user_data);
void on_button_stop_clicked (GtkButton *button, gpointer user_data);
void on_button_next_clicked (GtkButton *button, gpointer user_data);
void on_button_eject_clicked (GtkButton *button, gpointer user_data);
void on_togglebutton_shuffle_toggled (GtkToggleButton *togglebutton, gpointer user_data);
void on_togglebutton_repeat_toggled (GtkToggleButton *togglebutton, gpointer user_data);
void on_togglebutton_equalizer_toggled (GtkToggleButton *togglebutton, gpointer user_data);

void on_treeview_playlist_row_activated (GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer user_data);
gboolean on_treeview_playlist_drag_drop (GtkWidget *wid, GdkDragContext *context, gint x, gint y, guint time, gpointer user_dat);
void on_treeview_playlist_drag_data_received (GtkWidget *wid, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_dat);
void on_button_add_clicked (GtkButton *button, gpointer user_data);
void on_button_remove_clicked (GtkButton *button, gpointer user_data);
void on_button_clear_clicked (GtkButton *button, gpointer user_data);

void cb_gap_player_tick (GAPPlayer *gp);
void sync_time (GtkAdjustment *adjust);
gboolean on_hscale_progress_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_hscale_progress_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_hscale_progress_motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
void on_hscale_progress_value_changed (GtkRange *range, gpointer user_data);
gboolean slider_moved_cb (GtkAdjustment *adjust);
gboolean changed_idle_cb (GtkRange *range);

gboolean cb_file_open (GtkDialog *dialog, int response_id, gboolean clear);
void cb_file_add (GtkWidget *widget, gpointer user_data);
void cb_playlist_load (GtkWidget *widget, gpointer user_data);

gboolean gap_add_files (char *title, GtkWindow *parent, gboolean clear_playlist);

gboolean on_window_main_drag_drop (GtkWidget *wid, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data);
void on_window_main_drag_data_received (GtkWidget *wid, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_data);

gboolean slider_dragging = FALSE;
gboolean slider_locked = FALSE;
long latest_set_time = -1;
long elapsed = 0;
guint slider_moved_timeout = 0;
guint value_changed_update_handler = 0;

static const GtkTargetEntry target_uri [] = {{ "text/uri-list", 0, 0 }};

gboolean gap_add_files (char *title, GtkWindow *parent, gboolean clear_playlist)
{
	GtkWidget *file_selector;
	int response;
	
	file_selector = gtk_file_chooser_dialog_new (title, parent,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_OK,
						NULL);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (file_selector), TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (file_selector), GTK_RESPONSE_OK);
	gtk_window_set_transient_for (GTK_WINDOW (file_selector), GTK_WINDOW (main_window));
	gtk_window_set_modal (GTK_WINDOW (file_selector), FALSE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (file_selector), TRUE);

/*	g_signal_connect (G_OBJECT (file_selector), "response", G_CALLBACK (cb_file_open), 
			parent);
	gtk_widget_show_all (file_selector); */
	response = gtk_dialog_run (GTK_DIALOG (file_selector));

	return cb_file_open (GTK_DIALOG (file_selector), response, clear_playlist);
}

void on_open1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if (gap_add_files (_("Open Audio Files..."), GTK_WINDOW (main_window), TRUE))
	{
		playlist_play_and_sel_first ();
	}
}

void on_open_location1_activate (GtkButton *button, gpointer user_data)
{
	GtkWidget *dialog, *entry;
	GladeXML *dialog_xml;
	const gchar *uri_entry;
	int response;
	GnomeVFSURI *vfsuri;

	dialog_xml = glade_xml_new (DATADIR "/gamp/open-location.glade",
					"dialog_open_location", NULL);
	dialog = glade_xml_get_widget (dialog_xml, "dialog_open_location");
	g_return_if_fail (dialog != NULL);
	entry = glade_xml_get_widget (dialog_xml, "entry_location");
	g_return_if_fail (entry != NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK)
	{
		uri_entry = gtk_entry_get_text (GTK_ENTRY (entry));
		if ((uri_entry != NULL) && (strcmp (uri_entry, "") != 0))
		{
			vfsuri = gnome_vfs_uri_new (uri_entry);
			g_printf ("uri_entry: %s\n", uri_entry);
			if ((vfsuri != NULL) && (gnome_vfs_uri_exists (vfsuri)))
			{
				playlist_clear ();

				if (gnome_vfs_uri_is_local (vfsuri))
				{
					char *filepath;
					char *t_artist = NULL, *t_title = NULL, *t_format = NULL;
					long t_duration;
					char *s_duration;

					filepath = gnome_vfs_get_local_path_from_uri (uri_entry);
					gap_get_metadata_uri (filepath, &t_artist, &t_title, &t_duration);
					t_format = g_strdup_printf ("%s - %s", t_artist, t_title);
					s_duration = g_strdup_printf ("%d:%02d", t_duration / 60, t_duration % 60);

					playlist_add_item (t_format, s_duration, filepath);
					g_free (t_format);
					g_free (t_artist);
					g_free (t_title);
					
				}
				else
					playlist_add_item (uri_entry, "0:00", uri_entry);
				
				playlist_play_and_sel_first ();
			}
			else
				error_dialog ("That location does not exist");
			gnome_vfs_uri_unref (vfsuri);
		}
	}
	gtk_widget_destroy (dialog);
	g_object_unref (dialog_xml);
}

void on_quit1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_main_quit ();
}

void on_open2_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gboolean files_added;
	
	files_added = gap_add_files (_("Open Playlist..."), GTK_WINDOW (main_window), TRUE);
}

void on_save1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
}

void on_clear1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	playlist_clear ();
}

void on_add_file1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gboolean files_added;
	
	files_added = gap_add_files (_("Open Audio Files..."), GTK_WINDOW (main_window), FALSE);
}

void on_remove_file1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	playlist_remove_selected (playlist_treeview);
}

void on_about1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	static GtkWidget *about = NULL;
	gchar *filename;
	GdkPixbuf *pixbuf = NULL;
	const gchar *authors[] = {
		"Mason Kidd <mrkidd@mrkidd.com>",
		NULL
	};
	const gchar *documenters[] = {
		NULL
	};
	const gchar *translators = _("translator_credits");
	const gchar *copyright = "Copyright \xc2\xa9 2004 Mason Kidd";
	
	if (about != NULL)
	{
		gtk_window_present (GTK_WINDOW (about));
		return;
	}

	filename = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
				"gamp.png", TRUE, NULL);
	if (filename != NULL)
	{
		pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
		g_free (filename);
	}

	about = gnome_about_new (PACKAGE, VERSION, copyright,
			_("A music player for Gnome using the Gstreamer libraries"),
			authors, documenters,
			strcmp (translators, "translator_credits") != 0 ? translators : NULL,
			pixbuf);
	if (pixbuf != NULL)
		g_object_unref (pixbuf);

	gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW (main_window));
	g_signal_connect (G_OBJECT (about), "destroy", G_CALLBACK (gtk_widget_destroyed), &about);
	gtk_widget_show (about);
}

gboolean on_window_main_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gtk_main_quit ();
	
	return FALSE;
}

void on_expander_playlist_activate (GtkExpander *expander, gpointer user_data)
{
	gtk_expander_set_use_underline (expander, TRUE);
	
	if (gtk_expander_get_expanded (expander))
		gtk_expander_set_label (expander, _("Show _Playlist"));
	else
		gtk_expander_set_label (expander, _("Hide _Playlist"));
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

gboolean
on_treeview_playlist_drag_drop (GtkWidget *wid, GdkDragContext *context, gint x, gint y, guint time, gpointer user_dat)
{
	GdkAtom target;
	
	g_print ("DnD signal received\n");

	target = gtk_drag_dest_find_target (wid, context, NULL);
	if (target != GDK_NONE)
	{
/*		gtk_drag_get_data (main_window, context, target, time); */
		
		return TRUE;
	}
	
	return FALSE;
}

void
on_treeview_playlist_drag_data_received (GtkWidget *wid, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_dat)
{
	GList *list = NULL, *p = NULL;
	GSList *file_list = NULL, *uris = NULL;
	char *mimetype;
	
	g_print ("DnD data received\n");

	if ((data->length >= 0) && (data->type == gdk_atom_intern ("text/uri-list", TRUE)))
	{
		list = gnome_vfs_uri_list_parse (data->data);
		p = list;
		while (p != NULL)
		{
			file_list = g_slist_prepend (file_list,
										gnome_vfs_uri_to_string ((const GnomeVFSURI *)(p->data),
										GNOME_VFS_URI_HIDE_NONE));
			p = g_list_next (p);
		}
		gnome_vfs_uri_list_free (list);
		file_list = g_slist_reverse (file_list);
		
		if (file_list == NULL)
			return;
			
		for (uris = file_list; uris; uris = uris->next)
		{
			mimetype = gnome_vfs_get_mime_type (uris->data);
			if (g_ascii_strcasecmp (mimetype, "audio/x-scpls") == 0)
				playlist_add_pls (uris->data);
			else
			{
				char *t_artist = NULL, *t_title = NULL, *t_format = NULL;
				long t_duration;
				char *s_duration;
				
				gap_get_metadata_uri (uris->data, &t_artist, &t_title, &t_duration);
				t_format = g_strdup_printf ("%s - %s", t_artist, t_title);
				s_duration = g_strdup_printf ("%d:%02d", t_duration / 60, t_duration % 60);
				
				playlist_add_item (t_format, s_duration, uris->data);
				g_free (t_format);
				g_free (t_artist);
				g_free (t_title);
			}
		}

		g_slist_foreach (file_list, (GFunc) g_free, NULL);
		g_slist_free (file_list);
		gtk_drag_finish (context, TRUE, FALSE, time);
	}
	
	gtk_drag_finish (context, FALSE, FALSE, time);
}

void on_button_add_clicked (GtkButton *button, gpointer user_data)
{
	gboolean files_added;
	
	files_added = gap_add_files (_("Open Audio Files..."), GTK_WINDOW (main_window), FALSE);
}

void on_button_remove_clicked (GtkButton *button, gpointer user_data)
{
	playlist_remove_selected (playlist_treeview);
}

void on_button_clear_clicked (GtkButton *button, gpointer user_data)
{
	playlist_clear ();
}

gboolean
cb_file_open (GtkDialog *dialog, int response_id, gboolean clear)
{
	char *mimetype;
	GSList *uri_list = NULL, *uris = NULL;
	char *selected_uri;
	gboolean file_added = FALSE;

	if (response_id != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));
		return FALSE;
	}

	uri_list = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
	if (uri_list == NULL)
	{
		/* Nothing was returned? */
		return FALSE;
	}	

	if (clear)
		playlist_clear ();
	
	for (uris = uri_list; uris; uris = uris->next)
	{
		mimetype = gnome_vfs_get_mime_type (uris->data);
		if (g_ascii_strcasecmp (mimetype, "audio/x-scpls") == 0)
			playlist_add_pls (uris->data);
		else
		{
			char *t_artist = NULL, *t_title = NULL, *t_format = NULL;
			long t_duration;
			char *s_duration;
		
			gap_get_metadata_uri (uris->data, &t_artist, &t_title, &t_duration);
			t_format = g_strdup_printf ("%s - %s", t_artist, t_title);
			s_duration = g_strdup_printf ("%d:%02d", t_duration / 60, t_duration % 60);

			playlist_add_item (t_format, s_duration, uris->data);
			file_added = TRUE;
			g_free (t_format);
			g_free (t_artist);
			g_free (t_title);
		}
	}

	g_slist_foreach (uri_list, (GFunc)g_free, NULL);
	g_slist_free (uri_list);
	gtk_widget_destroy (GTK_WIDGET (dialog));
	
	return file_added;
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

void update_time (int time)
{
	char *current_time;
	int seconds = 0, minutes = 0;
	
	if (time > 0)
	{
		minutes = time / 60;
		seconds = time % 60;
	}
	current_time = g_strdup_printf ("<span size=\"xx-large\"><b>%d:%02d</b></span>", minutes, seconds);
	gtk_label_set_markup (GTK_LABEL (label_time), current_time);
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
	gtk_label_set_line_wrap (GTK_LABEL (label_current_song), FALSE);
	gtk_label_set_markup (GTK_LABEL (label_current_song), currently_playing);

	g_free (currently_playing);	
}

void cb_gap_player_eos (GAPPlayer *gp)
{
	playlist_next ();
}

void cb_gap_player_tick (GAPPlayer *gp)
{
	sync_time (gtk_range_get_adjustment (GTK_RANGE (glade_xml_get_widget (xml, "hscale_progress"))));
}

void
sync_time (GtkAdjustment *adjust)
{
	int seconds;
	long duration;
	
	if (slider_dragging == TRUE)
		return;
		
	duration = gap_get_duration (gamp_gp);
	seconds = gap_get_time (gamp_gp);
	
	if (duration > -1)
	{
		double progress = 0.0;
		
		if (seconds > 0)
			progress = (double) ((long) seconds) / duration;
			
		slider_locked = TRUE;
		gtk_adjustment_set_value (adjust, progress);
		slider_locked = FALSE;
	}
	else
	{
		slider_locked = TRUE;
		gtk_adjustment_set_value (adjust, 0.0);
		slider_locked = FALSE;
	}
	update_time (seconds);
}

gboolean
on_hscale_progress_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	slider_dragging = TRUE;
	
	return FALSE;
}

gboolean
on_hscale_progress_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	GtkAdjustment *adjust;
	long duration;
	double progress, new;
	
	if (slider_dragging == FALSE)
		return;
	
	adjust = gtk_range_get_adjustment (GTK_RANGE (widget));
	progress = gtk_adjustment_get_value (adjust);
	duration = gap_get_duration (gamp_gp);
	new = (long) (progress * duration);
	gap_set_time (gamp_gp, new);
	
	slider_dragging = FALSE;
	
	return FALSE;
}

gboolean
on_hscale_progress_motion_notify_event (GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	return FALSE;
}

void
on_hscale_progress_value_changed (GtkRange *range, gpointer user_data)
{
}

gboolean
changed_idle_cb (GtkRange *range)
{
	return FALSE;
}

gboolean
slider_moved_cb (GtkAdjustment *adjust)
{
	return FALSE;
}

gboolean
on_window_main_drag_drop (GtkWidget *wid, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data)
{
	GdkAtom target;
	
	g_print ("DnD signal received\n");

	target = gtk_drag_dest_find_target (wid, context, NULL);
	if (target != GDK_NONE)
	{
/*		gtk_drag_get_data (main_window, context, target, time); */
		
		return TRUE;
	}
	
	return FALSE;
}

void
on_window_main_drag_data_received (GtkWidget *wid, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_data)
{
	GList *list = NULL, *p = NULL;
	GSList *file_list = NULL, *uris = NULL;
	char *mimetype;
	
	g_print ("DnD data received\n");

	if ((data->length >= 0) && (data->type == gdk_atom_intern ("text/uri-list", TRUE)))
	{
		list = gnome_vfs_uri_list_parse (data->data);
		p = list;
		while (p != NULL)
		{
			file_list = g_slist_prepend (file_list,
										gnome_vfs_uri_to_string ((const GnomeVFSURI *)(p->data),
										GNOME_VFS_URI_HIDE_NONE));
			p = g_list_next (p);
		}
		gnome_vfs_uri_list_free (list);
		file_list = g_slist_reverse (file_list);
		
		if (file_list == NULL)
			return;
			
		playlist_clear ();

		for (uris = file_list; uris; uris = uris->next)
		{
			mimetype = gnome_vfs_get_mime_type (uris->data);
			if (g_ascii_strcasecmp (mimetype, "audio/x-scpls") == 0)
				playlist_add_pls (uris->data);
			else
			{
				char *t_artist = NULL, *t_title = NULL, *t_format = NULL;
				long t_duration;
				char *s_duration;
				
				gap_get_metadata_uri (uris->data, &t_artist, &t_title, &t_duration);
				t_format = g_strdup_printf ("%s - %s", t_artist, t_title);
				s_duration = g_strdup_printf ("%d:%02d", t_duration / 60, t_duration % 60);
				
				playlist_add_item (t_format, s_duration, uris->data);
				g_free (t_format);
				g_free (t_artist);
				g_free (t_title);
			}
		}

		playlist_play_and_sel_first ();
		
		g_slist_foreach (file_list, (GFunc) g_free, NULL);
		g_slist_free (file_list);
		gtk_drag_finish (context, TRUE, FALSE, time);
	}
	
	gtk_drag_finish (context, FALSE, FALSE, time);
}

