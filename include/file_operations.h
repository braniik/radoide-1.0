#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <gtk/gtk.h>
#include "main.h"

void populate_tree(GtkTreeStore *store, GtkTreeIter *parent, const gchar *path);
void refresh_current_folder(void);
void remove_no_files_placeholder(GtkTreeIter *parent);
void add_to_tree(const gchar *name, const gchar *full_path, gboolean is_dir);

void on_open_clicked(GtkWidget *widget, gpointer data);
void on_open_folder_clicked(GtkWidget *widget, gpointer data);
void on_save_clicked(GtkWidget *widget, gpointer data);
void on_save_current_clicked(GtkWidget *widget, gpointer data);
void on_new_c_file_clicked(GtkWidget *widget, gpointer data);
void on_new_text_file_clicked(GtkWidget *widget, gpointer data);
void on_new_folder_clicked(GtkWidget *widget, gpointer data);
void on_exit_clicked(GtkWidget *widget, gpointer data);

#endif