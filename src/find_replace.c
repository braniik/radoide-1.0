#include "find_replace.h"
#include "main.h"
#include "styling.h"

void clear_highlights() {
    if (!text_buffer) {
        g_print("Warning: text_buffer is NULL in clear_highlights\n");
        return;
    }
    
    GtkTextBuffer *buffer = text_buffer;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_remove_tag_by_name(buffer, "search_highlight", &start, &end);
}

void highlight_matches(const gchar *search_text) {
    g_print("highlight_matches called - search_text: %p\n", search_text);
    
    if (!text_buffer) {
        g_print("Error: text_buffer is NULL in highlight_matches\n");
        return;
    }
    if (!search_text) {
        g_print("Warning: search_text is NULL in highlight_matches\n");
        return;
    }
    if (!*search_text) {
        g_print("Warning: search_text is empty in highlight_matches\n");
        clear_highlights();
        return;
    }
    
    GtkTextBuffer *buffer = text_buffer;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    
    clear_highlights();
    
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY | GTK_TEXT_SEARCH_VISIBLE_ONLY;
    GtkTextIter match_start = start, match_end;
    gboolean found_any = FALSE;
    
    while (gtk_text_iter_forward_search(&match_start, search_text, flags, &match_start, &match_end, &end)) {
        gtk_text_buffer_apply_tag_by_name(buffer, "search_highlight", &match_start, &match_end);
        match_start = match_end;
        found_any = TRUE;
    }
    
    g_print("highlight_matches completed - found: %d\n", found_any);
}

gboolean find_next(const gchar *search_text, gboolean forward) {
    g_print("find_next called - search_text: %p, forward: %d\n", search_text, forward);
    
    if (!text_buffer) {
        g_print("Error: text_buffer is NULL in find_next\n");
        return FALSE;
    }
    if (!search_text) {
        g_print("Error: search_text is NULL in find_next\n");
        return FALSE;
    }
    if (!*search_text) {
        g_print("Error: search_text is empty in find_next\n");
        return FALSE;
    }
    
    GtkTextBuffer *buffer = text_buffer;
    GtkTextIter start_iter, end_iter;
    
    if (forward) {
        if (last_found_pos && gtk_text_buffer_get_mark(buffer, "last_found_pos")) {
            gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, last_found_pos);
            gtk_text_iter_forward_char(&start_iter);
        } else {
            gtk_text_buffer_get_start_iter(buffer, &start_iter);
        }
        gtk_text_buffer_get_end_iter(buffer, &end_iter);
    } else {
        if (last_found_pos && gtk_text_buffer_get_mark(buffer, "last_found_pos")) {
            gtk_text_buffer_get_iter_at_mark(buffer, &start_iter, last_found_pos);
        } else {
            gtk_text_buffer_get_end_iter(buffer, &start_iter);
        }
        gtk_text_buffer_get_start_iter(buffer, &end_iter);
    }
    
    GtkTextSearchFlags flags = GTK_TEXT_SEARCH_TEXT_ONLY | GTK_TEXT_SEARCH_VISIBLE_ONLY;
    GtkTextIter match_start, match_end;
    gboolean found;
    
    if (forward) {
        found = gtk_text_iter_forward_search(&start_iter, search_text, flags, &match_start, &match_end, &end_iter);
    } else {
        found = gtk_text_iter_backward_search(&start_iter, search_text, flags, &match_start, &match_end, &end_iter);
    }
    
    if (found) {
        gtk_text_buffer_select_range(buffer, &match_start, &match_end);
        
        if (global_source_view && GTK_IS_TEXT_VIEW(global_source_view)) {
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(global_source_view), &match_start, 0.0, FALSE, 0.0, 0.0);
        }
        
        if (last_found_pos) {
            if (gtk_text_buffer_get_mark(buffer, "last_found_pos")) {
                gtk_text_buffer_delete_mark(buffer, last_found_pos);
            }
            last_found_pos = NULL;
        }
        last_found_pos = gtk_text_buffer_create_mark(buffer, "last_found_pos", &match_start, TRUE);
        g_print("find_next found match\n");
        return TRUE;
    }
    
    if (last_found_pos) {
        if (gtk_text_buffer_get_mark(buffer, "last_found_pos")) {
            gtk_text_buffer_delete_mark(buffer, last_found_pos);
        }
        last_found_pos = NULL;
    }
    g_print("find_next no match found\n");
    return FALSE;
}

gboolean on_find_entry_changed_timeout(gpointer user_data) {
    g_print("on_find_entry_changed_timeout called\n");
    
    if (!find_entry) {
        g_print("Warning: find_entry is NULL in timeout\n");
        return G_SOURCE_REMOVE;
    }
    
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(find_entry));
    g_print("Timeout - search_text: '%s'\n", search_text ? search_text : "NULL");
    highlight_matches(search_text);
    find_timeout_id = 0;
    return G_SOURCE_REMOVE;
}

void on_find_entry_changed(GtkEntry *entry, gpointer user_data) {
    g_print("on_find_entry_changed called\n");
    
    if (find_timeout_id) {
        g_source_remove(find_timeout_id);
    }
    find_timeout_id = g_timeout_add(300, on_find_entry_changed_timeout, NULL);
}

void on_find_next_clicked(GtkWidget *widget, gpointer user_data) {
    g_print("Find next clicked\n");
    
    if (!find_entry) {
        g_print("Error: find_entry is NULL\n");
        return;
    }
    
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(find_entry));
    g_print("Find next - search_text: '%s'\n", search_text ? search_text : "NULL");
    
    if (!search_text || !*search_text) {
        g_print("No search text entered\n");
        return;
    }
    
    if (!find_next(search_text, TRUE)) {
        g_print("No more matches found, wrapping around\n");
        if (last_found_pos) {
            if (text_buffer && gtk_text_buffer_get_mark(text_buffer, "last_found_pos")) {
                gtk_text_buffer_delete_mark(text_buffer, last_found_pos);
            }
            last_found_pos = NULL;
            find_next(search_text, TRUE);
        }
    }
}

void on_find_prev_clicked(GtkWidget *widget, gpointer user_data) {
    g_print("Find previous clicked\n");
    
    if (!find_entry) {
        g_print("Error: find_entry is NULL\n");
        return;
    }
    
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(find_entry));
    g_print("Find prev - search_text: '%s'\n", search_text ? search_text : "NULL");
    
    if (!search_text || !*search_text) {
        g_print("No search text entered\n");
        return;
    }
    
    if (!find_next(search_text, FALSE)) {
        g_print("No more matches found, wrapping around\n");
        if (last_found_pos) {
            if (text_buffer && gtk_text_buffer_get_mark(text_buffer, "last_found_pos")) {
                gtk_text_buffer_delete_mark(text_buffer, last_found_pos);
            }
            last_found_pos = NULL;
            find_next(search_text, FALSE);
        }
    }
}

void on_find_close_clicked(GtkWidget *widget, gpointer user_data) {
    g_print("Find close clicked\n");
    
    GtkWidget *find_container = GTK_WIDGET(user_data);
    if (!find_container) {
        g_print("Error: find_container is NULL\n");
        return;
    }
    
    if (find_timeout_id) {
        g_source_remove(find_timeout_id);
        find_timeout_id = 0;
    }
    
    gtk_widget_hide(find_container);
    clear_highlights();
    if (last_found_pos) {
        if (text_buffer && gtk_text_buffer_get_mark(text_buffer, "last_found_pos")) {
            gtk_text_buffer_delete_mark(text_buffer, last_found_pos);
        }
        last_found_pos = NULL;
    }
    
    if (find_entry) {
        g_signal_handlers_block_by_func(find_entry, on_find_entry_changed, NULL);
        gtk_entry_set_text(GTK_ENTRY(find_entry), "");
        g_signal_handlers_unblock_by_func(find_entry, on_find_entry_changed, NULL);
    }
}

void on_find_clicked(GtkWidget *widget, gpointer user_data) {
    g_print("Find clicked - text_buffer: %p\n", text_buffer);
    
    GtkWidget *find_container = GTK_WIDGET(user_data);
    if (!find_container) {
        g_print("Error: find_container is NULL\n");
        return;
    }
    
    if (!text_buffer) {
        g_print("Error: Text buffer not available for find operation\n");
        return;
    }
    
    if (gtk_widget_get_visible(find_container)) {
        g_print("Hiding find container\n");
        
        if (find_timeout_id) {
            g_source_remove(find_timeout_id);
            find_timeout_id = 0;
        }
        
        gtk_widget_hide(find_container);
        clear_highlights();
        if (last_found_pos) {
            if (text_buffer && gtk_text_buffer_get_mark(text_buffer, "last_found_pos")) {
                gtk_text_buffer_delete_mark(text_buffer, last_found_pos);
            }
            last_found_pos = NULL;
        }
        
        if (find_entry) {
            g_signal_handlers_block_by_func(find_entry, on_find_entry_changed, NULL);
            gtk_entry_set_text(GTK_ENTRY(find_entry), "");
            g_signal_handlers_unblock_by_func(find_entry, on_find_entry_changed, NULL);
        }
    } else {
        g_print("Showing find container\n");
        gtk_widget_show_all(find_container);
        if (find_entry) {
            gtk_widget_grab_focus(find_entry);
        }
    }
}