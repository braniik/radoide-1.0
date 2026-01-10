#include "main.h"
#include "styling.h"
#include "motivation.h"
#include "terminal.h"
#include "file_operations.h"
#include "file_tree.h"
#include "editor.h"
#include "find_replace.h"
#include "build_system.h"


GtkTextBuffer *text_buffer = NULL;
GtkWidget *global_tree_view = NULL;
GtkTreeStore *global_tree_store = NULL;
GtkTreeIter *global_root_iter = NULL;
gchar *current_folder_path = NULL;
gchar *project_root_path = NULL;
GtkTreeIter *project_root_iter = NULL;
GtkTreeIter *current_folder_iter = NULL;
GtkWidget *terminal_box;
GtkWidget *terminal_text_view;
GtkTextBuffer *terminal_buffer;
GtkWidget *compile_button;
GtkWidget *run_button;
GtkWidget *terminal_toggle_button;
gboolean terminal_visible = FALSE;
gchar *current_file_path = NULL;
GtkWidget *find_bar = NULL;
GtkWidget *find_entry = NULL;
GtkWidget *find_prev_btn = NULL;
GtkWidget *find_next_btn = NULL;
GtkWidget *find_close_btn = NULL;
GtkTextMark *last_found_pos = NULL;
guint find_timeout_id = 0;
GtkWidget *global_source_view = NULL;

TerminalData *global_terminal = NULL;
GtkWidget *motivation_button = NULL;
GtkWidget *motivation_window = NULL;
GtkWidget *motivation_image = NULL;

void populate_tree(GtkTreeStore *store, GtkTreeIter *parent, const gchar *path);
void refresh_current_folder();
void remove_no_files_placeholder(GtkTreeIter *parent);
void terminal_send_command(const gchar *command);

void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *source_view;
    GtkWidget *scrolled_window;
    GtkWidget *vbox;
    GtkWidget *menu_bar;
    GtkWidget *file_menu;
    GtkWidget *file_menu_item;
    GtkWidget *open_item;
    GtkWidget *open_folder_item;
    GtkWidget *save_item;
    GtkWidget *exit_item;
    GtkWidget *tree_view;
    GtkWidget *side_panel;
    GtkCssProvider *css_provider;
    GdkDisplay *display;
    GdkScreen *screen;
    
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "RadoIDE");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    
    GError *icon_error = NULL;
    GdkPixbuf *icon = gdk_pixbuf_new_from_resource("/com/radoking/radoide/radoidelogo.jpg", &icon_error);
    if (icon) {
        gtk_window_set_icon(GTK_WINDOW(window), icon);
        g_object_unref(icon);
    } else {
        g_warning("Failed to load icon from resources: %s", icon_error ? icon_error->message : "Unknown error");
        if (icon_error) g_error_free(icon_error);
    }
    
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
    
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    file_menu = gtk_menu_new();
    file_menu_item = gtk_menu_item_new_with_label("File");
    open_item = gtk_menu_item_new_with_label("Open");
    open_folder_item = gtk_menu_item_new_with_label("Open Folder");
    GtkWidget *save_current_item = gtk_menu_item_new_with_label("Save");
    save_item = gtk_menu_item_new_with_label("Save As");
    exit_item = gtk_menu_item_new_with_label("Exit");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_folder_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_current_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), exit_item);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_menu_item);
    
    // New menu
    GtkWidget *new_menu = gtk_menu_new();
    GtkWidget *new_menu_item = gtk_menu_item_new_with_label("New");
    GtkWidget *new_folder_item = gtk_menu_item_new_with_label("Folder");
    GtkWidget *new_c_file_item = gtk_menu_item_new_with_label("C File");
    GtkWidget *new_text_file_item = gtk_menu_item_new_with_label("Text File");
    gtk_menu_shell_append(GTK_MENU_SHELL(new_menu), new_folder_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(new_menu), new_c_file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(new_menu), new_text_file_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_menu_item), new_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), new_menu_item);
    
    // Build menu
    GtkWidget *build_menu = gtk_menu_new();
    GtkWidget *build_menu_item = gtk_menu_item_new_with_label("Build");
    compile_button = gtk_menu_item_new_with_label("Compile");
    run_button = gtk_menu_item_new_with_label("Run");
    terminal_toggle_button = gtk_menu_item_new_with_label("Toggle Terminal");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(build_menu), compile_button);
    gtk_menu_shell_append(GTK_MENU_SHELL(build_menu), run_button);
    gtk_menu_shell_append(GTK_MENU_SHELL(build_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(build_menu), terminal_toggle_button);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(build_menu_item), build_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), build_menu_item);
    
    GtkWidget *find_menu_item = gtk_menu_item_new_with_label("Find");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), find_menu_item);

    // RADOKING MOTIVATION BUTTON LET'S GOOOOOOOO1!!!!!1!!!!!1!!!
    GtkWidget *motivation_button = gtk_menu_item_new_with_label("Motivation");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), motivation_button);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    side_panel = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(side_panel), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(side_panel, 250, -1);
    
    GtkTreeStore *store = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
    tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    global_tree_store = store;
    global_tree_view = tree_view;
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Files", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    GtkTreeIter iter;
    gtk_tree_store_append(store, &iter, NULL);
    gtk_tree_store_set(store, &iter, 0, "Project Files", -1);
    global_root_iter = gtk_tree_iter_copy(&iter);
    populate_tree(store, &iter, ".");
    gtk_container_add(GTK_CONTAINER(side_panel), tree_view);
    gtk_box_pack_start(GTK_BOX(hbox), side_panel, FALSE, FALSE, 0);
    g_signal_connect(open_folder_item, "activate", G_CALLBACK(on_open_folder_clicked), tree_view);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Syntax highlighting setup
    GtkSourceLanguageManager *lm = gtk_source_language_manager_get_default();
    GtkSourceLanguage *c_lang = gtk_source_language_manager_get_language(lm, "c");
    
    GtkSourceBuffer *source_buffer = gtk_source_buffer_new_with_language(c_lang);
    text_buffer = GTK_TEXT_BUFFER(source_buffer);

    GtkSourceStyleSchemeManager *scheme_manager = gtk_source_style_scheme_manager_get_default();
    GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(scheme_manager, "cobalt");
    if (scheme) {
        gtk_source_buffer_set_style_scheme(source_buffer, scheme);
    }

    // Search highlight tag
    GtkTextTag *highlight_tag = gtk_text_buffer_create_tag(text_buffer, "search_highlight", "background", "yellow","foreground", "black", NULL);

    // Signal for bracket indentation
    g_signal_connect(source_buffer, "insert-text", G_CALLBACK(on_bracket_inserted_after), NULL);

    // Source view
    source_view = gtk_source_view_new_with_buffer(source_buffer);
    global_source_view = source_view;
    
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(source_view), 4);
    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_indent_on_tab(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_show_line_marks(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_smart_home_end(GTK_SOURCE_VIEW(source_view), GTK_SOURCE_SMART_HOME_END_AFTER);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(source_view), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(source_view), TRUE);

    // Find bar container
    GtkWidget *find_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(find_container, GTK_ALIGN_END);
    gtk_widget_set_margin_top(find_container, 5);
    gtk_widget_set_margin_bottom(find_container, 5);
    gtk_widget_set_name(find_container, "find-bar");
    
    // Find
    GtkWidget *find_label = gtk_label_new("Find:");
    gtk_box_pack_start(GTK_BOX(find_container), find_label, FALSE, FALSE, 0);
    
    find_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(find_entry), "Type to search...");
    gtk_widget_set_size_request(find_entry, 200, -1);
    gtk_box_pack_start(GTK_BOX(find_container), find_entry, FALSE, FALSE, 0);
    
    find_prev_btn = gtk_button_new_with_label("▲");
    gtk_widget_set_tooltip_text(find_prev_btn, "Find Previous");
    gtk_widget_set_name(find_prev_btn, "find-prev-btn");
    gtk_box_pack_start(GTK_BOX(find_container), find_prev_btn, FALSE, FALSE, 0);
    
    find_next_btn = gtk_button_new_with_label("▼");
    gtk_widget_set_tooltip_text(find_next_btn, "Find Next");
    gtk_widget_set_name(find_next_btn, "find-next-btn");
    gtk_box_pack_start(GTK_BOX(find_container), find_next_btn, FALSE, FALSE, 0);
    
    find_close_btn = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(find_close_btn, "Close Find");
    gtk_widget_set_name(find_close_btn, "find-close-btn");
    gtk_box_pack_start(GTK_BOX(find_container), find_close_btn, FALSE, FALSE, 0);

    // Direct style because CSS bugs
    style_button_directly(find_prev_btn, "#322f2f", "#ffffff");
    style_button_directly(find_next_btn, "#322f2f", "#ffffff");
    style_button_directly(find_close_btn, "#322f2f", "#ff0404");
    
    gtk_box_pack_start(GTK_BOX(vbox), find_container, FALSE, FALSE, 0);
    gtk_widget_hide(find_container);

    // Connect tree view signals
    g_signal_connect(tree_view, "row-activated", G_CALLBACK(on_file_clicked), text_buffer);
    g_signal_connect(tree_view, "key-press-event", G_CALLBACK(on_tree_key_press), window);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    g_signal_connect(selection, "changed", G_CALLBACK(on_tree_selection_changed), NULL);

    // Connect find bar signals
    g_signal_connect(find_entry, "changed", G_CALLBACK(on_find_entry_changed), NULL);
    g_signal_connect(find_next_btn, "clicked", G_CALLBACK(on_find_next_clicked), NULL);
    g_signal_connect(find_prev_btn, "clicked", G_CALLBACK(on_find_prev_clicked), NULL);
    g_signal_connect(find_close_btn, "clicked", G_CALLBACK(on_find_close_clicked), find_container);
    g_signal_connect(find_menu_item, "activate", G_CALLBACK(on_find_clicked), find_container);

    // CSS    
    css_provider = gtk_css_provider_new();
    const gchar *css_data = 
    "window {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "}"
    "vbox {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "}"
    "hbox {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "}"
    "menubar {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "}"
    "menubar > menuitem {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "   padding: 4px 8px;"
    "}"
    "menubar > menuitem:hover {"
    "   background-color: #0078D4;"
    "   color: #FFFFFF;"
    "}"
    "menu {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "   border: 1px solid #3e3e42;"
    "}"
    "menu > menuitem {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "   padding: 6px 12px;"
    "}"
    "menu > menuitem:hover {"
    "   background-color: #0078D4;"
    "   color: #FFFFFF;"
    "}"
    "menu > separator {"
    "   background-color: #3e3e42;"
    "   margin: 4px 0;"
    "}"
    "scrolledwindow {"
    "   background-color: #000000;"
    "}"
    "treeview {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "   font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
    "   font-size: 10pt;"
    "}"
    "treeview:selected {"
    "   background-color: #0078D4;"
    "   color: #FFFFFF;"
    "}"
    "treeview:hover {"
    "   background-color: #1a1a1a;"
    "}"
    "treeview header {"
    "   background-color: #353535;"
    "   color: #FFFFFF;"
    "}"
    "treeview header button {"
    "   background-color: #353535;"
    "   color: #FFFFFF;"
    "}"
    "textview {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "   font-family: 'Consolas', 'Courier New', monospace;"
    "   font-size: 12pt;"
    "   caret-color: #FFFFFF;"
    "}"
    "textview text {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "   caret-color: #FFFFFF;"
    "}"
    "textview border {"
    "   background-color: #353535;"
    "   color: #ffffff;"
    "}"
    ".terminal textview {"
    "   background-color: #000000;"
    "   color: #FFFFFF;"
    "   font-family: 'Consolas', 'Courier New', monospace;"
    "   font-size: 11pt;"
    "}"
    ".terminal-header {"
    "   background-color: #1a1a1a;"
    "   color: #FFFFFF;"
    "   border-bottom: 1px solid #3e3e42;"
    "   font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
    "   font-weight: bold;"
    "}"
    "#terminal-input-box {"
    "   background-color: #1a1a1a;"
    "   padding: 1px 3px;"
    "   border-top: 1px solid #3e3e42;"
    "   min-height: 24px;"
    "}"
    "#terminal-entry {"
    "   background-color: #2d2d30;"
    "   color: #FFFFFF;"
    "   border: 1px solid #3e3e42;"
    "   font-family: 'Consolas', 'Courier New', monospace;"
    "   font-size: 9pt;"
    "   padding: 1px 4px;"
    "   min-height: 20px;"
    "}"
    "#terminal-prompt {"
    "   color: #FFFFFF;"
    "   font-family: 'Consolas', 'Courier New', monospace;"
    "   font-size: 9pt;"
    "   padding: 1px 2px;"
    "}"
    "button {"
    "   background-color: #0e639c;"
    "   color: #ffffff;"
    "   border: none;"
    "   border-radius: 2px;"
    "   padding: 6px 12px;"
    "   font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
    "}"
    "button:hover {"
    "   background-color: #1177bb;"
    "}"
    "button:active {"
    "   background-color: #005a9e;"
    "}"
    "button:disabled {"
    "   background-color: #3c3c3c;"
    "   color: #666666;"
    "}"
    "label {"
    "   color: #FFFFFF;"
    "   font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
    "}"
    "entry {"
    "   background-color: #2d2d30;"
    "   color: #FFFFFF;"
    "   border: 1px solid #3e3e42;"
    "   border-radius: 2px;"
    "   padding: 6px 8px;"
    "   font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
    "}"
    "entry:focus {"
    "   border-color: #0078D4;"
    "}"
    "scrollbar {"
    "   background-color: #2d2d30;"
    "}"
    "scrollbar slider {"
    "   background-color: #484848;"
    "}"
    "scrollbar slider:hover {"
    "   background-color: #5a5a5a;"
    "}"
    "scrollbar slider:active {"
    "   background-color: #0078D4;"
    "}"
    "scrollbar trough {"
    "   background-color: #2d2d30;"
    "}"
    "separator {"
    "   background-color: #3e3e42;"
    "}"
    "dialog {"
    "   background-color: #1e1e1e;"
    "   color: #FFFFFF;"
    "   border: 1px solid #3e3e42;"
    "}"
    "messagedialog {"
    "   background-color: #1e1e1e;"
    "   color: #FFFFFF;"
    "}"
    "tooltip {"
    "   background-color: #2d2d30;"
    "   color: #FFFFFF;"
    "   border: 1px solid #3e3e42;"
    "}"
    "*:focus {"
    "   outline: 1px solid #0078D4;"
    "}"
    "#find-bar {"
    "   background-color: #2d2d30;"
    "   color: #ffffff;"
    "   border: 1px solid #3e3e42;"
    "   border-radius: 3px;"
    "   padding: 1px 3px;"
    "   margin: 1px 3px;"
    "   max-width: 350px;"
    "}"
    "#find-bar entry {"
    "   background-color: #3c3c3c;"
    "   color: #ffffff;"
    "   border: 1px solid #0078d4;"
    "   border-radius: 2px;"
    "   padding: 1px 4px;"
    "   width: 120px;"
    "   min-height: 20px;"
    "   font-size: 0.8em;"
    "}"
    "#find-prev-btn {"
    "   background-color: #0e639c;"
    "   color: #ffffff;"
    "   border: none;"
    "   border-radius: 2px;"
    "   padding: 1px 4px;"
    "   min-width: 24px;"
    "   min-height: 20px;"
    "   font-size: 0.7em;"
    "}"
    "#find-next-btn {"
    "   background-color: #0e639c;"
    "   color: #ffffff;"
    "   border: none;"
    "   border-radius: 2px;"
    "   padding: 1px 4px;"
    "   min-width: 24px;"
    "   min-height: 20px;"
    "   font-size: 0.7em;"
    "}"
    "#find-close-btn {"
    "   background-color: #2c2d2e;"
    "   color: #ffffff;"
    "   border: none;"
    "   border-radius: 2px;"
    "   padding: 1px 4px;"
    "   min-width: 24px;"
    "   min-height: 20px;"
    "   font-size: 0.7em;"
    "}"
    "#find-prev-btn:hover {"
    "   background-color: #1177bb;"
    "}"
    "#find-next-btn:hover {"
    "   background-color: #1177bb;"
    "}"
    "#find-close-btn:hover {"
    "   background-color: #1177bb;"
    "}"
    "#find-prev-btn:active {"
    "   background-color: #005a9e;"
    "}"
    "#find-next-btn:active {"
    "   background-color: #005a9e;"
    "}"
    "#find-close-btn:active {"
    "   background-color: #005a9e;"
    "}"
    "#find-bar label {"
    "   color: #cccccc;"
    "   padding: 1px 4px;"
    "   font-size: 0.8em;"
    "}";

    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_container_add(GTK_CONTAINER(scrolled_window), source_view);
    gtk_box_pack_start(GTK_BOX(hbox), scrolled_window, TRUE, TRUE, 0);

    // Terminal section
    terminal_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(terminal_box, -1, 200);

    GtkWidget *terminal_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    GtkWidget *terminal_label = gtk_label_new("Terminal Output");
    gtk_box_pack_start(GTK_BOX(terminal_header), terminal_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(terminal_box), terminal_header, FALSE, FALSE, 0);

    GtkWidget *terminal = create_terminal();
    gtk_box_pack_start(GTK_BOX(terminal_box), terminal, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), terminal_box, FALSE, TRUE, 0);
    gtk_widget_hide(terminal_box);

    // Other signals
    g_signal_connect(exit_item, "activate", G_CALLBACK(on_exit_clicked), window);
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open_clicked), window);
    g_signal_connect(save_current_item, "activate", G_CALLBACK(on_save_current_clicked), window);
    g_signal_connect(save_item, "activate", G_CALLBACK(on_save_clicked), window);
    g_signal_connect(compile_button, "activate", G_CALLBACK(on_compile_clicked), NULL);
    g_signal_connect(run_button, "activate", G_CALLBACK(on_run_clicked), NULL);
    g_signal_connect(terminal_toggle_button, "activate", G_CALLBACK(on_terminal_toggle_clicked), NULL);
    g_signal_connect(new_folder_item, "activate", G_CALLBACK(on_new_folder_clicked), window);
    g_signal_connect(new_c_file_item, "activate", G_CALLBACK(on_new_c_file_clicked), window);
    g_signal_connect(new_text_file_item, "activate", G_CALLBACK(on_new_text_file_clicked), window);
    g_signal_connect(motivation_button, "activate", G_CALLBACK(on_motivation_clicked), window);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    
    app = gtk_application_new("com.radoking.radoide", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}