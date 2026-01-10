#ifndef STYLING_H
#define STYLING_H

#include <gtk/gtk.h>

void style_dialog_buttons(GtkWidget *dialog);
void style_button_directly(GtkWidget *button, const gchar *bg_color, const gchar *fg_color);

#endif