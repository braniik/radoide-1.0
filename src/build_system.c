#include "build_system.h"
#include "main.h"
#include "terminal.h"

void show_terminal_message(const gchar *message);

void on_compile_clicked(GtkWidget *widget, gpointer data) {
    if (!current_file_path) {
        if (global_terminal) {
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(global_terminal->buffer, &iter);
            gtk_text_buffer_insert(global_terminal->buffer, &iter, "Error: No file open to compile\n", -1);
        }
        return;
    }
    
    gchar *extension = g_strrstr(current_file_path, ".");
    if (!extension || (g_ascii_strcasecmp(extension, ".c") != 0)) {
        if (global_terminal) {
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(global_terminal->buffer, &iter);
            gtk_text_buffer_insert(global_terminal->buffer, &iter, "Error: Not a C file\n", -1);
        }
        return;
    }
    
    if (!terminal_visible) {
        gtk_widget_show_all(terminal_box);
        terminal_visible = TRUE;
    }
    
    gchar *dir_path = g_path_get_dirname(current_file_path);
    gchar *base_name = g_path_get_basename(current_file_path);
    gchar *exe_name = g_strdup(base_name);
    gchar *dot = g_strrstr(exe_name, ".");
    if (dot) *dot = '\0';
    
    gchar *unix_path = g_strdup(dir_path);
    for (gchar *p = unix_path; *p; p++) {
        if (*p == '\\') *p = '/';
    }
    
    gchar *cd_command = g_strdup_printf("cd \"%s\"", unix_path);
    terminal_send_command(cd_command);
    
    gchar *compile_command = g_strdup_printf("gcc -o %s.exe %s", exe_name, base_name);
    terminal_send_command(compile_command);
    
    g_free(dir_path);
    g_free(base_name);
    g_free(exe_name);
    g_free(cd_command);
    g_free(compile_command);
    g_free(unix_path);
}

void on_run_clicked(GtkWidget *widget, gpointer data) {
    if (!current_file_path) {
        if (global_terminal) {
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(global_terminal->buffer, &iter);
            gtk_text_buffer_insert(global_terminal->buffer, &iter, "Error: No file selected\n", -1);
        }
        return;
    }
    
    if (!terminal_visible) {
        gtk_widget_show_all(terminal_box);
        terminal_visible = TRUE;
    }
    
    gchar *dir_path = g_path_get_dirname(current_file_path);
    gchar *base_name = g_path_get_basename(current_file_path);
    gchar *exe_name = g_strdup(base_name);
    gchar *dot = g_strrstr(exe_name, ".");
    if (dot) *dot = '\0';
    
    gchar *unix_path = g_strdup(dir_path);
    for (gchar *p = unix_path; *p; p++) {
        if (*p == '\\') *p = '/';
    }
    
    gchar *cd_command = g_strdup_printf("cd \"%s\"", unix_path);
    terminal_send_command(cd_command);
    
    gchar *run_command = g_strdup_printf("./%s.exe", exe_name);
    terminal_send_command(run_command);
    
    g_free(dir_path);
    g_free(base_name);
    g_free(exe_name);
    g_free(unix_path);
    g_free(cd_command);
    g_free(run_command);
}

void on_terminal_toggle_clicked(GtkWidget *widget, gpointer data) {
    if (terminal_visible) {
        gtk_widget_hide(terminal_box);
        terminal_visible = FALSE;
    } else {
        gtk_widget_show_all(terminal_box);
        terminal_visible = TRUE;
        if (global_terminal && global_terminal->input_entry) {
            gtk_widget_grab_focus(global_terminal->input_entry);
        }
    }
}