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

int main (int argc, char *argv[])
{
	GnomeProgram *p;
	poptContext context;
	const gchar **argvn;
	char *mime_type;
	gint i;
	
	#ifdef ENABLE_NLS
		bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
		textdomain (PACKAGE);
	#endif
	
	static const struct poptOption options[] = {
		  {NULL, '\0', 0, NULL, 0, NULL, NULL}
	};

	p = gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv, GNOME_PARAM_POPT_TABLE, options, NULL);
	glade_gnome_init ();
	gst_init (&argc, &argv);
	gnome_vfs_init ();
	/*
	 * The .glade filename should be on the next line.
	 */
	xml = glade_xml_new (PACKAGE_SOURCE_DIR"/gamp.glade", NULL, NULL);

	gamp_gp = gap_player_new();
	
	/* This is important */
	glade_xml_signal_autoconnect (xml);
	main_window = glade_xml_get_widget (xml, "window1");
	playlist_window = glade_xml_get_widget (xml, "window_playlist");
	gtk_widget_show (main_window);

	playlist_treeview = GTK_TREE_VIEW (glade_xml_get_widget (xml, "treeview_playlist"));
	playlist_create_list(playlist_treeview);
	
	label_current_song = glade_xml_get_widget (xml, "label_current_song");
	g_signal_connect (G_OBJECT (gamp_gp), "eos", G_CALLBACK (cb_gap_player_eos), NULL);
	
	g_object_get (p, "popt-context", &context, NULL);
	argvn = poptGetArgs (context);
	if (argvn)
	{
		i = 0;
		while (argvn[i] != NULL)
		{
			mime_type = gnome_vfs_get_mime_type (argvn[i]);
			if (g_ascii_strcasecmp (mime_type, "audio/x-scpls") == 0)
				playlist_add_pls (argvn[i]);
			else
				playlist_add_item (argvn[i], "0:00", argvn[i]);
			i++;
		}
		//playlist_play_and_sel_first ();
	}
	
	gtk_main ();
	
	g_object_unref (gamp_gp);
	
	return 0;
}
