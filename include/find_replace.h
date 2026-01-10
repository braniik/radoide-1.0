#ifndef FIND_REPLACE_H
#define FIND_REPLACE_H

#include <gtk/gtk.h>
#include "main.h"

void on_find_clicked(GtkWidget *widget, gpointer data);
void on_find_next_clicked(GtkWidget *widget, gpointer user_data);
void on_find_prev_clicked(GtkWidget *widget, gpointer user_data);
void on_find_close_clicked(GtkWidget *widget, gpointer user_data);
void on_find_entry_changed(GtkEntry *entry, gpointer user_data);
gboolean on_find_entry_changed_timeout(gpointer user_data);
gboolean find_next(const gchar *search_text, gboolean forward);
void highlight_matches(const gchar *search_text);
void clear_highlights(void);

#endif