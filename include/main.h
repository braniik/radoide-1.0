#ifndef MAIN_H
#define MAIN_H

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <windows.h>
#include <commdlg.h>
#include <direct.h>
#include <sys/stat.h>
#include <shlobj.h>
#include <process.h>

extern GtkTextBuffer *text_buffer;
extern GtkWidget *global_tree_view;
extern GtkTreeStore *global_tree_store;
extern GtkTreeIter *global_root_iter;
extern gchar *project_root_path;
extern GtkTreeIter *project_root_iter;
extern gchar *current_folder_path;
extern GtkTreeIter *current_folder_iter;
extern GtkWidget *terminal_box;
extern GtkWidget *terminal_text_view;
extern GtkTextBuffer *terminal_buffer;
extern GtkWidget *compile_button;
extern GtkWidget *run_button;
extern GtkWidget *terminal_toggle_button;
extern gboolean terminal_visible;
extern gchar *current_file_path;
extern GtkWidget *find_bar;
extern GtkWidget *find_entry;
extern GtkWidget *find_prev_btn;
extern GtkWidget *find_next_btn;
extern GtkWidget *find_close_btn;
extern GtkTextMark *last_found_pos;
extern guint find_timeout_id;
extern GtkWidget *global_source_view;

typedef struct {
    HANDLE hInputWrite;
    HANDLE hOutputRead;
    HANDLE hProcess;
    GIOChannel *channel;
    guint watch_id;
    GtkTextBuffer *buffer;
    GtkWidget *text_view;
    GtkWidget *input_entry;
} TerminalData;

extern TerminalData *global_terminal;
extern GtkWidget *motivation_button;
extern GtkWidget *motivation_window;
extern GtkWidget *motivation_image;

int main(int argc, char **argv);
void on_activate(GtkApplication *app, gpointer user_data);

#endif