#include "file_operations.h"
#include "main.h"
#include "file_tree.h"
#include "styling.h"
#include "terminal.h"
#include "file_tree.h"

void remove_no_files_placeholder(GtkTreeIter *parent) {
    if (global_tree_store == NULL || parent == NULL) return;
    
    GtkTreeIter child;
    if (gtk_tree_model_iter_children(GTK_TREE_MODEL(global_tree_store), &child, parent)) {
        gchar *child_name = NULL;
        gtk_tree_model_get(GTK_TREE_MODEL(global_tree_store), &child, 0, &child_name, -1);
        
        if (child_name && (g_strcmp0(child_name, "(No Files Yet)") == 0)) {
            gtk_tree_store_remove(global_tree_store, &child);
        }
        
        if (child_name) g_free(child_name);
    }
}

void refresh_current_folder() {
    if (current_folder_iter && current_folder_path) {
        GtkTreeIter parent_iter = *current_folder_iter;
        
        GtkTreeIter child;
        while (gtk_tree_model_iter_children(GTK_TREE_MODEL(global_tree_store), &child, &parent_iter)) {
            gtk_tree_store_remove(global_tree_store, &child);
        }
        
        populate_tree(global_tree_store, &parent_iter, current_folder_path);
    }
}

void add_to_tree(const gchar *name, const gchar *full_path, gboolean is_dir) {
    if (global_tree_store == NULL) return;
    
    GtkTreeIter iter;
    GtkTreeIter *parent = current_folder_iter ? current_folder_iter : global_root_iter;

    if (parent != NULL) {
        remove_no_files_placeholder(parent);
        
        gtk_tree_store_append(global_tree_store, &iter, parent);
        gtk_tree_store_set(global_tree_store, &iter, 0, name, 1, full_path, 2, is_dir, -1);
        
        if (is_dir) {
            GtkTreeIter child;
            gtk_tree_store_append(global_tree_store, &child, &iter);
            gtk_tree_store_set(global_tree_store, &child, 0, "(No Files Yet)", -1);
        }
    }
}

void on_new_c_file_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("New C File", GTK_WINDOW(data), GTK_DIALOG_MODAL, "Create", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);

    style_dialog_buttons(dialog);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "main.c");
    gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 5);
    gtk_widget_show_all(dialog);
    
    const gchar *original_filename = NULL;
    gchar *allocated_filename = NULL;
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        original_filename = gtk_entry_get_text(GTK_ENTRY(entry));
        const gchar *filename_to_use;
        
        if (original_filename == NULL || strlen(original_filename) == 0) {
            filename_to_use = "main.c";
        } else {
            gchar *dot = g_strrstr(original_filename, ".");
            if (dot == NULL) {
                allocated_filename = g_strdup_printf("%s.c", original_filename);
                filename_to_use = allocated_filename;
            } else {
                filename_to_use = original_filename;
            }
        }
        
        const gchar *base_path = current_folder_path ? current_folder_path : ".";
        gchar *filepath = g_build_filename(base_path, filename_to_use, NULL);
        
        FILE *file = fopen(filepath, "w");
        if (file) {
            fprintf(file, "#include <stdio.h>\n\n");
            fprintf(file, "int main() {\n");
            fprintf(file, "    printf(\"Hello, World!\\n\");\n");
            fprintf(file, "    return 0;\n");
            fprintf(file, "}\n");
            fclose(file);
            
            add_to_tree(filename_to_use, filepath, FALSE);
            refresh_current_folder();
            
            gchar *content = NULL;
            gsize length;
            if (g_file_get_contents(filepath, &content, &length, NULL)) {
                gtk_text_buffer_set_text(text_buffer, content, -1);
                g_free(content);
            }
        } else {
            g_print("Failed to create file: %s\n", filepath);
        }
        g_free(filepath);
        
        if (allocated_filename) {
            g_free(allocated_filename);
        }
    }
    gtk_widget_destroy(dialog);
}

void on_new_text_file_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("New Text File", GTK_WINDOW(data), GTK_DIALOG_MODAL, "Create", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);

    style_dialog_buttons(dialog);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "text.txt");
    gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 5);
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar *filename = gtk_entry_get_text(GTK_ENTRY(entry));
        if (filename == NULL || strlen(filename) == 0) {
            filename = "text.txt";
        }
        
        const gchar *base_path = current_folder_path ? current_folder_path : ".";
        gchar *filepath = g_build_filename(base_path, filename, NULL);
        
        FILE *file = fopen(filepath, "w");
        if (file) {
            fclose(file);
            
            add_to_tree(filename, filepath, FALSE);
            refresh_current_folder();
            
            gtk_text_buffer_set_text(text_buffer, "", -1);
        } else {
            g_print("Failed to create file: %s\n", filepath);
        }
        g_free(filepath);
    }
    gtk_widget_destroy(dialog);
}

void on_new_folder_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("New Folder", GTK_WINDOW(data), GTK_DIALOG_MODAL, "Create", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);

    style_dialog_buttons(dialog);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "newfolder");
    gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 5);
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar *foldername = gtk_entry_get_text(GTK_ENTRY(entry));
        if (foldername == NULL || strlen(foldername) == 0) {
            foldername = "newfolder";
        }
        
        const gchar *base_path = current_folder_path ? current_folder_path : (project_root_path ? project_root_path : ".");
        
        gchar *folderpath = g_build_filename(base_path, foldername, NULL);
        
        g_print("Creating directory at: %s\n", folderpath);
        g_print("Base path: %s\n", base_path);
        
        if (mkdir(folderpath) == 0) {
            add_to_tree(foldername, folderpath, TRUE);
            refresh_current_folder();
            g_print("Successfully created directory: %s\n", folderpath);
        } else {
            g_print("Failed to create directory: %s (error: %d)\n", folderpath, errno);
            if (project_root_path && strcmp(base_path, project_root_path) != 0) {
                g_free(folderpath);
                folderpath = g_build_filename(project_root_path, foldername, NULL);
                g_print("Trying fallback location: %s\n", folderpath);
                if (mkdir(folderpath) == 0) {
                    add_to_tree(foldername, folderpath, TRUE);
                    refresh_current_folder();
                    g_print("Successfully created directory in fallback: %s\n", folderpath);
                }
            }
        }
        g_free(folderpath);
    }
    gtk_widget_destroy(dialog);
}

void on_exit_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *window = GTK_WIDGET(data);
    gtk_widget_destroy(window);
}

void on_open_clicked(GtkWidget *widget, gpointer data) {
    OPENFILENAME ofn;
    char szFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0C Files\0*.C;*.CPP;*.H\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        gchar *content = NULL;
        gsize length;
        GError *error = NULL;
        
        if (g_file_get_contents(szFile, &content, &length, &error)) {
            gtk_text_buffer_set_text(text_buffer, content, -1);
            gchar *dir_path = g_path_get_dirname(szFile);
            g_print("Opened file from: %s\n", dir_path);
            g_free(content);
            g_free(dir_path);
            
            if (current_file_path) g_free(current_file_path);
            current_file_path = g_strdup(szFile);
        }
    }
}

void on_open_folder_clicked(GtkWidget *widget, gpointer data) {
    BROWSEINFO bi = {0};
    char path[MAX_PATH];
    
    bi.lpszTitle = "Select Project Folder";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    
    if (pidl != NULL) {
        if (SHGetPathFromIDList(pidl, path)) {
            if (project_root_path) g_free(project_root_path);
            if (project_root_iter) gtk_tree_iter_free(project_root_iter);
            if (current_folder_path) g_free(current_folder_path);
            if (current_folder_iter) gtk_tree_iter_free(current_folder_iter);
            
            project_root_path = g_strdup(path);
            
            GtkTreeStore *store = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
            global_tree_store = store;
            GtkTreeView *tree_view = GTK_TREE_VIEW(data);
            gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(store));
            
            GtkTreeIter iter;
            gtk_tree_store_append(store, &iter, NULL);
            gtk_tree_store_set(store, &iter, 0, path, 1, path, 2, TRUE, -1);
            
            project_root_iter = gtk_tree_iter_copy(&iter);
            current_folder_iter = gtk_tree_iter_copy(&iter);
            current_folder_path = g_strdup(path);
            
            populate_tree(store, &iter, path);
            
            g_print("Opened project: %s\n", path);
            g_print("Project root: %s\n", project_root_path);
            g_print("Current folder: %s\n", current_folder_path);
        }
        CoTaskMemFree(pidl);
    }
}

void on_save_clicked(GtkWidget *widget, gpointer data) {
    OPENFILENAME ofn;
    char szFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0C Files\0*.C\0Header Files\0*.H\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileName(&ofn)) {
        GtkTextIter start, end; 
        gchar *text;
        gtk_text_buffer_get_start_iter(text_buffer, &start);
        gtk_text_buffer_get_end_iter(text_buffer, &end);
        
        text = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);
        GError *error = NULL;
        if (!g_file_set_contents(szFile, text, -1, &error)) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(data), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Failed to save file: %s", error->message);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            g_error_free(error);
        }
        g_free(text);
    }
}

void on_save_current_clicked(GtkWidget *widget, gpointer data) {
    if (current_file_path && g_file_test(current_file_path, G_FILE_TEST_EXISTS)) {
        GtkTextIter start, end;
        gchar *text;
        
        gtk_text_buffer_get_start_iter(text_buffer, &start);
        gtk_text_buffer_get_end_iter(text_buffer, &end);
        text = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);
        
        GError *error = NULL;
        if (!g_file_set_contents(current_file_path, text, -1, &error)) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(data), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Failed to save file: %s", error->message);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            g_error_free(error);
        } else {
            g_print("File saved: %s\n", current_file_path);
        }
        g_free(text);
    } else {
        g_print("No existing file, using Save As...\n");
        on_save_clicked(widget, data);
    }
}

void populate_tree(GtkTreeStore *store, GtkTreeIter *parent, const gchar *path) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    gchar searchPath[MAX_PATH];
    
    snprintf(searchPath, sizeof(searchPath), "%s\\*", path);
    
    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (findFileData.cFileName[0] == '.' || strcmp(findFileData.cFileName, "..") == 0) {
                continue;
            }
            
            gchar *full_path = g_build_filename(path, findFileData.cFileName, NULL);
            
            gboolean is_dir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
            g_print("File: %s, Attributes: 0x%x, IsDir: %d\n", findFileData.cFileName, findFileData.dwFileAttributes, is_dir);
            
            GtkTreeIter iter;
            gtk_tree_store_append(store, &iter, parent);
            gtk_tree_store_set(store, &iter, 0, findFileData.cFileName, 1, full_path, 2, is_dir, -1);
            
            if (is_dir) {
                populate_tree(store, &iter, full_path);
                
                GtkTreeIter child;
                if (!gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &child, &iter)) {
                    gtk_tree_store_append(store, &child, &iter);
                    gtk_tree_store_set(store, &child, 0, "(No Files Yet)", -1);
                }
            }
            
            g_free(full_path);
        } while (FindNextFile(hFind, &findFileData));
        
        FindClose(hFind);
    }
}