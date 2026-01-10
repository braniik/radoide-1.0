#include "motivation.h"
#include "main.h"

// Yes, this function is so important I am making and entire file for it :)

void on_motivation_clicked(GtkWidget *widget, gpointer user_data) {
    if (motivation_window && gtk_widget_get_visible(motivation_window)) {
        gtk_widget_hide(motivation_window);
        return;
    }
    
    if (!motivation_window) {
        motivation_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(motivation_window), "Motivation!");
        gtk_window_set_default_size(GTK_WINDOW(motivation_window), 200, 200);
        gtk_window_set_position(GTK_WINDOW(motivation_window), GTK_WIN_POS_CENTER);
        gtk_window_set_decorated(GTK_WINDOW(motivation_window), FALSE);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(motivation_window), TRUE);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(motivation_window), TRUE);
        gtk_window_set_keep_above(GTK_WINDOW(motivation_window), TRUE);
        
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(motivation_window), box);
        motivation_image = gtk_image_new_from_resource("/com/radoking/radoide/radoidelogo.jpg");
        
        GError *img_error = NULL;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_resource("/com/radoking/radoide/radoidelogo.jpg", &img_error);

        if (pixbuf) {
            motivation_image = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
        } else {
            g_warning("Failed to load motivation image: %s", img_error ? img_error->message : "Unknown error");
            if (img_error) g_error_free(img_error);
            motivation_image = gtk_label_new("RADOKING");
            gtk_label_set_justify(GTK_LABEL(motivation_image), GTK_JUSTIFY_CENTER);
        }

        gtk_widget_set_hexpand(motivation_image, TRUE);
        gtk_widget_set_vexpand(motivation_image, TRUE);
        
        gtk_box_pack_start(GTK_BOX(box), motivation_image, TRUE, TRUE, 0);
        
        g_signal_connect_swapped(motivation_window, "button-press-event", G_CALLBACK(gtk_widget_hide), motivation_window);
        g_signal_connect_swapped(motivation_window, "key-press-event", G_CALLBACK(gtk_widget_hide), motivation_window);
    }
    
    GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(motivation_window));
    gint screen_width = gdk_screen_get_width(screen);
    gint screen_height = gdk_screen_get_height(screen);
    
    gint window_width, window_height;
    gtk_window_get_size(GTK_WINDOW(motivation_window), &window_width, &window_height);
    
    gtk_window_move(GTK_WINDOW(motivation_window), screen_width - window_width - 20, 20);
    
    gtk_widget_show_all(motivation_window);
    gtk_widget_grab_focus(motivation_window);
}