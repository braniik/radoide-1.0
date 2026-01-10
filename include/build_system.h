#ifndef BUILD_SYSTEM_H
#define BUILD_SYSTEM_H

#include <gtk/gtk.h>
#include "main.h"

void on_compile_clicked(GtkWidget *widget, gpointer data);
void on_run_clicked(GtkWidget *widget, gpointer data);
void on_terminal_toggle_clicked(GtkWidget *widget, gpointer data);

#endif