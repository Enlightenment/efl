/*
 * Copyright (C) 2011 Igalia S.L.
 * Authors: Mario Sanchez Prada <msanchez@igalia.com>
 *
 * Based on a C# example written in C# by Mike Gorse:
 * http://mgorse.freeshell.org/plug-socket-test.tar.gz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 3 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include "ta-plug-vbox.h"

#include <gtk/gtk.h>

#define WINDOW_WIDTH 350
#define WINDOW_HEIGHT -1

static void
_button_clicked_cb (GtkButton *button, gpointer data)
{
  GtkWindow *window = GTK_WINDOW (data);

  GtkWidget *dialog =
    gtk_message_dialog_new (window,
                            GTK_DIALOG_MODAL,
                            GTK_MESSAGE_INFO,
                            GTK_BUTTONS_CLOSE,
                            "I told you so:\nthe button does ALMOST nothing",
                            NULL);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

int
main (int argc, char**argv)
{
  GtkWidget *window = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *plug = NULL;
  GtkWidget *widget = NULL;
  gchar *plug_id = NULL;
  gchar *entry_txt = NULL;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  plug = ta_plug_vbox_new (FALSE, 0);
  plug_id = ta_plug_vbox_get_id (TA_PLUG_VBOX (plug));
  if (plug_id)
    g_print ("[PLUG] Id is %s\n", plug_id);
  else
    g_print ("Not a valid ID found\n");


  hbox = gtk_hbox_new (FALSE, 0);

  widget = gtk_label_new ("Plug ID: ");
  gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 6);

  widget = gtk_entry_new ();
  entry_txt = g_strdup_printf ("%s", plug_id);
  gtk_entry_set_text (GTK_ENTRY (widget), entry_txt);
  gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 6);

  gtk_box_pack_start (GTK_BOX (plug), hbox, TRUE, TRUE, 6);

  widget = gtk_button_new_with_label ("A button which does almost nothing");
  g_signal_connect (widget, "clicked", G_CALLBACK (_button_clicked_cb), window);
  gtk_box_pack_start (GTK_BOX (plug), widget, FALSE, FALSE, 6);

  g_free (entry_txt);
  g_free (plug_id);

  gtk_container_add (GTK_CONTAINER (window), plug);

  gtk_window_set_default_size (GTK_WINDOW (window),
                               WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_widget_show_all (window);
  gtk_main ();
}
