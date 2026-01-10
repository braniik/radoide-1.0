#ifndef TERMINAL_H
#define TERMINAL_H

#include <gtk/gtk.h>
#include "main.h"

GtkWidget* create_terminal(void);
TerminalData* create_msys2_terminal(GtkWidget *text_view, GtkTextBuffer *buffer);
void terminal_send_command(const gchar *command);
gboolean terminal_output_callback(GIOChannel *channel, GIOCondition condition, gpointer data);
void on_terminal_input_activate(GtkEntry *entry, gpointer user_data);

#endif