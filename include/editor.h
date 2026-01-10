#ifndef EDITOR_H
#define EDITOR_H

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include "main.h"

void on_bracket_inserted_after(GtkTextBuffer *buffer, GtkTextIter *location, const gchar *text, gint len, gpointer user_data);

#endif