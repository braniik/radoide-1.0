#include "styling.h"
#include "main.h"

// FCK CSS
void style_dialog_buttons(GtkWidget *dialog) {
    GtkWidget *action_area = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    
    GtkStyleContext *area_context = gtk_widget_get_style_context(action_area);
    GtkCssProvider *area_provider = gtk_css_provider_new();
    
    const gchar *area_css = 
        "* { background-color: #1e1e1e; padding: 5px; }";

    gtk_css_provider_load_from_data(area_provider, area_css, -1, NULL);
    gtk_style_context_add_provider(area_context, GTK_STYLE_PROVIDER(area_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(area_provider);
    
    GList *children = gtk_container_get_children(GTK_CONTAINER(action_area));
    
    int i = 0;
    
    for (GList *iter = children; iter != NULL; iter = iter->next, i++) {
        GtkWidget *button = GTK_WIDGET(iter->data);
        if (GTK_IS_BUTTON(button)) {
            gchar *button_name = g_strdup_printf("dialog-button-%d", i);
            gtk_widget_set_name(button, button_name);
            
            GtkStyleContext *context = gtk_widget_get_style_context(button);
            GtkCssProvider *provider = gtk_css_provider_new();
            
            gchar *css = g_strdup_printf(
                "#%s { "
                "background-image: none; "
                "background: #3c3c3c; "
                "color: white; "
                "border: 1px solid #555555; "
                "border-radius: 3px; "
                "min-height: 15px; "
                "min-width: 30px; "
                "padding: 6px 12px; "
                "font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; "
                "font-size: 8pt; "
                "box-shadow: none; "
                "} "
                "#%s:hover { background: #4a4a4a; border-color: #666666; } "
                "#%s:active { background: #2a2a2a; }",
                button_name, button_name, button_name);
            
            gtk_css_provider_load_from_data(provider, css, -1, NULL);
            gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            
            g_free(css);
            g_free(button_name);
            g_object_unref(provider);
            
            g_print("Button %d styled gray with proper font\n", i);
        }
    }
    g_list_free(children);
}

// CSS is retarded in GTK, that's why I am adding afunction to force button colors
void style_button_directly(GtkWidget *button, const gchar *bg_color, const gchar *fg_color) {
    GtkStyleContext *context = gtk_widget_get_style_context(button);
    GtkCssProvider *provider = gtk_css_provider_new();
    
    gchar *css = g_strdup_printf(
        "* { "
        "background-image: none; "
        "background-color: %s; "
        "color: %s; "
        "border: none; "
        "border-width: 0; "
        "border-style: none; "
        "outline: none; "
        "box-shadow: none; "
        "padding: 2px 6px; "
        "}", 
        bg_color, fg_color
    );
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_free(css);
    g_object_unref(provider);
}