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
#include "ta-socket-label.h"

#include <gtk/gtk.h>

#define WINDOW_WIDTH 300
#define WINDOW_HEIGHT -1

static GtkWidget *entry = NULL;
static GtkWidget *button = NULL;
static GtkWidget *socket = NULL;

static void
_button_clicked_cb (GtkButton *button, gpointer data)
{
  AtkObject *socketAxObject = NULL;
  gchar *text_id = NULL;

  text_id = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

  if (text_id && text_id[0] != '\0')
    {
      socketAxObject = gtk_widget_get_accessible (socket);

      if (ATK_IS_SOCKET (socketAxObject))
        {
          g_print ("[SOCKET] Embedding object with ID %s\n", text_id);
          atk_socket_embed (ATK_SOCKET (socketAxObject), text_id);
          g_print ("[SOCKET] Done\n");
        }
      else
        g_print ("Not embedding anything: Not an instance of AtkSocket\n");
    }
  else
    g_print ("Not embedding: you need to provide an ID\n");

  g_free (text_id);
}

static void
_create_widgets (GtkWidget *window)
{
  GtkWidget *vbox = NULL;

  vbox = gtk_vbox_new (FALSE, 0);

  /* Label + entry */
  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 6);

  button = gtk_button_new_with_label ("Connect to plug");
  g_signal_connect (button, "clicked", G_CALLBACK (_button_clicked_cb), NULL);
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 6);

  /* Our socket accessible */
  socket = ta_socket_label_new ("Socket accessible goes here");
  gtk_box_pack_start (GTK_BOX (vbox), socket, FALSE, FALSE, 6);

  gtk_container_add (GTK_CONTAINER (window), vbox);
}

int
main (int argc, char**argv)
{
  GtkWidget *window = NULL;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  _create_widgets (window);

  gtk_window_set_default_size (GTK_WINDOW (window),
                               WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_widget_show_all (window);
  gtk_main ();
}
