#include <gnome.h>
#include <glade/glade.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "main.h"
#include "playlist.h"
#include "GAPPlayer.h"

void on_open1_activate (GtkMenuItem *menuitem, gpointer user_data);
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

void on_button_open_clicked (GtkButton *button, gpointer user_data);
void on_button_cancel_clicked (GtkButton *button, gpointer user_data);
void on_open_location1_activate (GtkButton *button, gpointer user_data);

void cb_file_open (GtkWidget *widget, gpointer user_data);

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
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tbutton), !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tbutton)));
	
	return TRUE;
}

void on_button_previous_clicked (GtkButton *button, gpointer user_data)
{
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

void on_togglebutton_equalizer_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
}

void on_togglebutton_playlist_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
	if (GTK_WIDGET_VISIBLE (playlist_window))
		gtk_widget_hide (playlist_window);
	else
		gtk_widget_show (playlist_window);
}

void on_button_open_clicked (GtkButton *button, gpointer user_data)
{
}

void on_button_cancel_clicked (GtkButton *button, gpointer user_data)
{
}

void on_open_location1_activate (GtkButton *button, gpointer user_data)
{
	GtkWidget *content, *entry_location, *dialog;
	GtkResponseType resp;
	GnomeVFSURI *vfsuri;
	char *uri_entry;
	
	dialog = gtk_dialog_new_with_buttons (_("Open Location"), GTK_WINDOW (main_window),
				GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	g_return_if_fail (dialog != NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	
	content = glade_xml_get_widget (xml, "vbox3");
	g_return_if_fail (content != NULL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), content, FALSE, FALSE, 0);
	
	entry_location = glade_xml_get_widget (xml, "entry_location");
	g_return_if_fail (entry_location != NULL);
	
	resp = gtk_dialog_run (GTK_DIALOG (dialog));
	if (resp == GTK_RESPONSE_OK)
	{
		uri_entry = gtk_editable_get_chars (GTK_EDITABLE (entry_location), 0, -1);
		if (uri_entry != NULL)
		{
			vfsuri = gnome_vfs_uri_new (uri_entry);
			if (vfsuri != NULL)
			{
				gap_open_location (gamp_gp, uri_entry);
				gnome_vfs_uri_unref (vfsuri);
			}
		}
	}
	gtk_widget_destroy (dialog);
}

void cb_file_open (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *file_selector;
	char *selected_uri;
	
	file_selector = (GtkWidget *) user_data;
	selected_uri = gnome_vfs_get_uri_from_local_path (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));
	g_print ("selected: %s\n", selected_uri);
	gap_open (gamp_gp, selected_uri);
	playlist_add_item (selected_uri, NULL);
}
