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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <glade/glade.h>
#include <gst/gst.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>

#include "main.h"
#include "playlist.h"
#include "GAPPlayer.h"


struct poptOption options[] = {
	{NULL, '\0', POPT_ARG_INCLUDE_TABLE, NULL, 0, "GStreamer", NULL},
	POPT_TABLEEND
};

char *uri_resolve_relative (const char *file);
static gboolean has_valid_scheme (const char *uri);
static gboolean is_valid_scheme_character (char c);
static char *file_uri_from_local_relative_path (const char *location);

static const GtkTargetEntry target_uri [] = {{ "text/uri-list", 0, 0 }};

static char *
file_uri_from_local_relative_path (const char *location)
{
	char *current_dir;
	char *base_uri, *base_uri_slash;
	char *location_escaped;
	char *uri;
	
	current_dir = g_get_current_dir ();
	base_uri = gnome_vfs_get_uri_from_local_path (current_dir);
	base_uri_slash = g_strconcat (base_uri, "/", NULL);
	
	location_escaped = gnome_vfs_escape_path_string (location);
	
	uri = g_strconcat (base_uri_slash, location_escaped, NULL);
	
	g_free (location_escaped);
	g_free (base_uri_slash);
	g_free (base_uri);
	g_free (current_dir);
	
	return uri;	
}

static gboolean
is_valid_scheme_character (char c)
{
	return g_ascii_isalnum (c) || c == '+' || c == '-' || c == '.';
}
        
static gboolean
has_valid_scheme (const char *uri)
{
	const char *p;
	
	p = uri;
	
	if (!is_valid_scheme_character (*p))
		return FALSE;
		
	do
	{
		p++;
	} while (is_valid_scheme_character (*p));
	
	return *p == ':';
}

char *
uri_resolve_relative (const char *file)
{
	char *uri;

	g_return_val_if_fail (file != NULL, g_strdup (""));

	switch (file[0])
	{
		case '\0':
			uri = g_strdup ("");
			break;
		case '/':
			uri = gnome_vfs_get_uri_from_local_path (file);
			break;
		default:
			if (has_valid_scheme (file))
				uri = g_strdup (file);
			else
				uri = file_uri_from_local_relative_path (file);
			break;
	}
	
	return uri;
}

void error_dialog (const char *error_text)
{
	GtkWidget *dialog, *label;
	gint response;
	
	dialog = gtk_dialog_new_with_buttons ("Gamp - Error", GTK_WINDOW (main_window),
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	label = gtk_label_new (error_text);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), label);
	
	response = gtk_dialog_run (GTK_DIALOG (dialog));
}

int
main (int argc, char *argv[])
{
	GnomeProgram *p;
	poptContext context;
	const gchar **argvn;
	char *mime_type;
	gint i;
	int win_width, win_height;
	
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (PACKAGE);
	
	options[0].arg = (void *) gst_init_get_popt_table ();
	if (!(p = gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
				argc, argv,
				GNOME_PARAM_POPT_TABLE,	options,
				GNOME_PARAM_HUMAN_READABLE_NAME, _("Gamp Audio Player"),
				GNOME_PARAM_APP_DATADIR, DATADIR,
				NULL)))
		g_error ("gnome_program_init failed");

	glade_gnome_init ();
	gnome_vfs_init ();

	xml = glade_xml_new (DATADIR "/gamp/gamp.glade", NULL, NULL);
	
	gamp_gp = gap_player_new();
	
	glade_xml_signal_autoconnect (xml);
	main_window = glade_xml_get_widget (xml, "window_main");
	gtk_drag_dest_set (main_window, GTK_DEST_DEFAULT_ALL, target_uri, 1, GDK_ACTION_COPY);

	gtk_widget_show_all (main_window);

	playlist_treeview = GTK_TREE_VIEW (glade_xml_get_widget (xml, "treeview_playlist"));
	playlist_create_list(playlist_treeview);
	gtk_drag_dest_set (GTK_WIDGET (playlist_treeview), GTK_DEST_DEFAULT_ALL, target_uri, 1, GDK_ACTION_COPY);
	
	label_current_song = glade_xml_get_widget (xml, "label_current_song");
	label_time = glade_xml_get_widget (xml, "label_time");
	g_signal_connect (G_OBJECT (gamp_gp), "eos", G_CALLBACK (cb_gap_player_eos), NULL);
	g_signal_connect (G_OBJECT (gamp_gp), "tick", G_CALLBACK (cb_gap_player_tick), NULL);
		
	g_object_get (p, "popt-context", &context, NULL);
	argvn = poptGetArgs (context);
	if (argvn)
	{
		i = 0;
		while (argvn[i] != NULL)
		{
			char *tmp;

			tmp = uri_resolve_relative (argvn[i]);
			mime_type = gnome_vfs_get_mime_type (tmp);
			g_print ("mime_type: %s\n", mime_type);
			if (g_ascii_strcasecmp (mime_type, "audio/x-scpls") == 0)
				playlist_add_pls (argvn[i]);
			else
			{
				char *t_artist = NULL, *t_title = NULL, *t_format = NULL;
				long t_duration;
				char *s_duration;
				
				gap_get_metadata_uri (argvn[i], &t_artist, &t_title, &t_duration);
				t_format = g_strdup_printf ("%s - %s", t_artist, t_title);
				s_duration = g_strdup_printf ("%d:%02d", t_duration / 60, t_duration %60);
				playlist_add_item (t_format, s_duration, argvn[i]);
				g_free (t_format);
				g_free (t_artist);
				g_free (t_title);
			}
			i++;
		}
		playlist_play_and_sel_first ();
	}
	
	gtk_main ();
	
	g_object_unref (gamp_gp);
	
	return 0;
}
