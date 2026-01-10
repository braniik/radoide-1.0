#include "terminal.h"
#include "main.h"

gboolean terminal_output_callback(GIOChannel *channel, GIOCondition condition, gpointer data) {
    TerminalData *term = (TerminalData *)data;
    gchar buffer[4096];
    gsize bytes_read;
    GError *error = NULL;
    
    if (condition & G_IO_IN) {
        GIOStatus status = g_io_channel_read_chars(channel, buffer, sizeof(buffer) - 1, &bytes_read, &error);
        
        if (status == G_IO_STATUS_NORMAL && bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(term->buffer, &iter);
            gtk_text_buffer_insert(term->buffer, &iter, buffer, -1);
            
            gtk_text_buffer_get_end_iter(term->buffer, &iter);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(term->text_view), &iter, 0.0, FALSE, 0.0, 0.0);
        }
        
        if (error) g_error_free(error);
    }
    
    if (condition & (G_IO_HUP | G_IO_ERR)) {
        return FALSE;
    }
    
    return TRUE;
}

TerminalData* create_msys2_terminal(GtkWidget *text_view, GtkTextBuffer *buffer) {
    TerminalData *term = g_new0(TerminalData, 1);
    term->buffer = buffer;
    term->text_view = text_view;
    
    HANDLE hInputRead, hOutputWrite;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    
    if (!CreatePipe(&hInputRead, &term->hInputWrite, &sa, 0) ||
        !CreatePipe(&term->hOutputRead, &hOutputWrite, &sa, 0)) {
        g_free(term);
        return NULL;
    }
    
    SetHandleInformation(term->hInputWrite, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(term->hOutputRead, HANDLE_FLAG_INHERIT, 0);
    
    const gchar *msys2_paths[] = {
        "C:\\msys64\\usr\\bin\\bash.exe",
        "C:\\msys32\\usr\\bin\\bash.exe",
        "C:\\tools\\msys64\\usr\\bin\\bash.exe",
        NULL
    };
    
    const gchar *bash_path = NULL;
    gboolean has_msys2 = FALSE;
    
    for (int i = 0; msys2_paths[i] != NULL; i++) {
        if (GetFileAttributes(msys2_paths[i]) != INVALID_FILE_ATTRIBUTES) {
            bash_path = msys2_paths[i];
            has_msys2 = TRUE;
            break;
        }
    }
    
    if (!bash_path) {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, 
            "Error: MSYS2 not found. Install to C:\\msys64\n"
            "Download: https://www.msys2.org/\n"
            "Falling back to cmd...\n\n", -1);
        bash_path = "cmd.exe";
    }
    
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = hInputRead;
    si.hStdOutput = hOutputWrite;
    si.hStdError = hOutputWrite;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    gchar *cmdline;
    
    if (has_msys2) {
        cmdline = g_strdup_printf(
            "\"%s\" --login -c \""
            "export PATH=/mingw64/bin:/usr/local/bin:/usr/bin:/bin:$PATH; "
            "export MSYSTEM=MINGW64; "
            "export TERM=dumb; "
            "export PS1='$ '; "
            "exec bash --norc --noprofile\"",
            bash_path
        );
    } else {
        cmdline = g_strdup("cmd.exe");
    }
    
    if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, CREATE_NEW_CONSOLE | CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        g_free(cmdline);
        CloseHandle(hInputRead);
        CloseHandle(hOutputWrite);
        CloseHandle(term->hInputWrite);
        CloseHandle(term->hOutputRead);
        g_free(term);
        return NULL;
    }
    
    g_free(cmdline);
    term->hProcess = pi.hProcess;
    CloseHandle(pi.hThread);
    CloseHandle(hInputRead);
    CloseHandle(hOutputWrite);
    
    int fd = _open_osfhandle((intptr_t)term->hOutputRead, 0);
    term->channel = g_io_channel_win32_new_fd(fd);
    g_io_channel_set_encoding(term->channel, NULL, NULL);
    g_io_channel_set_buffered(term->channel, FALSE);
    g_io_channel_set_flags(term->channel, G_IO_FLAG_NONBLOCK, NULL);
    
    term->watch_id = g_io_add_watch(term->channel, G_IO_IN | G_IO_HUP | G_IO_ERR, terminal_output_callback, term);
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, 
        "=== RadoIDE Integrated Terminal ===\n"
        "MSYS2 bash initialized. Type commands or use Build menu.\n\n", -1);
    return term;
}

void terminal_send_command(const gchar *command) {
    if (!global_terminal || !global_terminal->hInputWrite) return;
    
    gchar *cmd_with_newline = g_strdup_printf("%s\n", command);
    DWORD written;
    WriteFile(global_terminal->hInputWrite, cmd_with_newline, strlen(cmd_with_newline), &written, NULL);
    g_free(cmd_with_newline);
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(global_terminal->buffer, &iter);
    gchar *display = g_strdup_printf("$ %s\n", command);
    gtk_text_buffer_insert(global_terminal->buffer, &iter, display, -1);
    g_free(display);
}

void on_terminal_input_activate(GtkEntry *entry, gpointer user_data) {
    const gchar *command = gtk_entry_get_text(entry);
    if (command && strlen(command) > 0) {
        terminal_send_command(command);
        gtk_entry_set_text(entry, "");
    }
}

GtkWidget* create_terminal() {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *text_view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_CHAR);
    
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_set_name(input_box, "terminal-input-box");
    GtkWidget *prompt_label = gtk_label_new("$");
    gtk_widget_set_name(prompt_label, "terminal-prompt");
    GtkWidget *input_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_entry), "Enter command...");
    gtk_widget_set_name(input_entry, "terminal-entry");
    gtk_widget_set_size_request(input_entry, -1, 20);
    gtk_box_pack_start(GTK_BOX(input_box), prompt_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(input_box), input_entry, TRUE, TRUE, 2);
    gtk_widget_set_size_request(input_box, -1, 24);
    gtk_box_pack_start(GTK_BOX(vbox), input_box, FALSE, FALSE, 2);
    
    global_terminal = create_msys2_terminal(text_view, buffer);
    if (global_terminal) {
        global_terminal->input_entry = input_entry;
        g_signal_connect(input_entry, "activate", G_CALLBACK(on_terminal_input_activate), NULL);
    }
    
    return vbox;
}