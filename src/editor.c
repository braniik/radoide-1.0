#include "editor.h"
#include "main.h"


void on_bracket_inserted_after(GtkTextBuffer *buffer, GtkTextIter *location, const gchar *text, gint len, gpointer user_data) {
    if (len != 1 || !text) return;
    
    gchar bracket = text[0];
    
    if (bracket != '{' && bracket != '(' && bracket != '[') return;
    
    g_signal_handlers_block_by_func(buffer, on_bracket_inserted_after, user_data);
    
    g_signal_stop_emission_by_name(buffer, "insert-text");
    
    if (bracket == '{') {
        GtkTextMark *insert_mark = gtk_text_buffer_get_insert(buffer);
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_mark(buffer, &iter, insert_mark);
        
        GtkTextIter line_start = iter;
        gtk_text_iter_set_line_offset(&line_start, 0);
        gchar *line_text = gtk_text_buffer_get_text(buffer, &line_start, &iter, FALSE);
        
        gint indent_count = 0;
        for (gchar *p = line_text; *p && (*p == ' ' || *p == '\t'); p++) {
            indent_count++;
        }
        gchar *indent = g_strndup(line_text, indent_count);
        gchar *inner_indent = g_strdup_printf("%s    ", indent);
        
        gchar *completion = g_strdup_printf("{\n%s\n%s}", inner_indent, indent);
        gtk_text_buffer_insert(buffer, &iter, completion, -1);
        
        gtk_text_buffer_get_iter_at_mark(buffer, &iter, insert_mark);
        gtk_text_iter_forward_chars(&iter, 1);
        gtk_text_iter_forward_char(&iter);
        gtk_text_buffer_place_cursor(buffer, &iter);
        
        g_free(line_text);
        g_free(indent);
        g_free(inner_indent);
        g_free(completion);
    }
    else {
        gchar closing = (bracket == '(') ? ')' : ']';
        
        GtkTextMark *insert_mark = gtk_text_buffer_get_insert(buffer);
        GtkTextIter iter;
        gtk_text_buffer_get_iter_at_mark(buffer, &iter, insert_mark);
        
        gchar both_brackets[3] = {bracket, closing, '\0'};
        gtk_text_buffer_insert(buffer, &iter, both_brackets, -1);
        
        gtk_text_buffer_get_iter_at_mark(buffer, &iter, insert_mark);
        gtk_text_iter_backward_char(&iter);
        gtk_text_buffer_place_cursor(buffer, &iter);
    }
    
    g_signal_handlers_unblock_by_func(buffer, on_bracket_inserted_after, user_data);
}
