#ifndef FILE_TREE_H
#define FILE_TREE_H

#include <gtk/gtk.h>
#include "main.h"

void on_file_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data);
gboolean on_tree_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

#endif