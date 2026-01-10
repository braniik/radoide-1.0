#include "file_tree.h"
#include "main.h"
#include "file_operations.h"
#include "styling.h"

void on_file_clicked(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *file_path;
    gboolean is_dir;
    
    model = gtk_tree_view_get_model(tree_view);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gtk_tree_model_get(model, &iter, 1, &file_path, 2, &is_dir, -1);
        
        if (file_path && g_file_test(file_path, G_FILE_TEST_EXISTS)) {
            if (is_dir) {
                if (current_folder_path) g_free(current_folder_path);
                if (current_folder_iter) gtk_tree_iter_free(current_folder_iter);
                
                current_folder_path = g_strdup(file_path);
                current_folder_iter = gtk_tree_iter_copy(&iter);
                
                g_print("Current folder set to: %s\n", current_folder_path);
                g_print("Project root is: %s\n", project_root_path ? project_root_path : "Not set");
            } else {
                gchar *content = NULL;
                gsize length;
                GError *error = NULL;
                
                if (g_file_get_contents(file_path, &content, &length, &error)) {
                    GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER(user_data);
                    gtk_text_buffer_set_text(text_buffer, content, -1);
                    g_free(content);
                    
                    if (current_file_path) g_free(current_file_path);
                    current_file_path = g_strdup(file_path);
                    g_print("Current file set to: %s\n", current_file_path);
                } else {
                    g_print("Failed to open file: %s\n", error ? error->message : "Unknown error");
                    if (error) g_error_free(error);
                }
            }
        } else {
            g_print("Path does not exist: %s\n", file_path);
        }
        g_free(file_path);
    }
}

void on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *file_path;
        gboolean is_dir;
        
        gtk_tree_model_get(model, &iter, 1, &file_path, 2, &is_dir, -1);
        
        if (is_dir) {
            if (current_folder_path) g_free(current_folder_path);
            if (current_folder_iter) gtk_tree_iter_free(current_folder_iter);
            
            current_folder_path = g_strdup(file_path);
            current_folder_iter = gtk_tree_iter_copy(&iter);
            g_print("Current folder set via selection: %s\n", current_folder_path);
        } else {
            if (current_file_path) g_free(current_file_path);
            current_file_path = g_strdup(file_path);
            g_print("Current file updated via selection: %s\n", current_file_path);
        }
        
        g_free(file_path);
    }
}

gboolean on_tree_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if (event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_BackSpace) {
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
        GtkTreeModel *model;
        GtkTreeIter iter;
        
        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
            gchar *file_path;
            gboolean is_dir;
            gchar *name;
            
            gtk_tree_model_get(model, &iter, 0, &name, 1, &file_path, 2, &is_dir, -1);
            
            if (g_strcmp0(name, "Project Files") == 0 || g_strcmp0(name, "(No Files Yet)") == 0) {
                g_free(name);
                g_free(file_path);
                return FALSE;
            }
            
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(user_data), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Are you sure you want to delete '%s'?", name);

            style_dialog_buttons(dialog);
            
            gtk_window_set_title(GTK_WINDOW(dialog), "Confirm Deletion");
            
            if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
                if (is_dir) {
                    SHFILEOPSTRUCT fileOp = {0};
                    fileOp.wFunc = FO_DELETE;
                    fileOp.pFrom = file_path;
                    fileOp.fFlags = FOF_NO_UI | FOF_ALLOWUNDO | FOF_NOCONFIRMATION;
                    
                    if (SHFileOperation(&fileOp) == 0) {
                        gtk_tree_store_remove(global_tree_store, &iter);
                        refresh_current_folder();
                    }
                } else {
                    if (remove(file_path) == 0) {
                        gtk_tree_store_remove(global_tree_store, &iter);
                        refresh_current_folder();
                    }
                }
            }
            
            gtk_widget_destroy(dialog);
            g_free(name);
            g_free(file_path);
            return TRUE;
        }
    }
    return FALSE;
}
