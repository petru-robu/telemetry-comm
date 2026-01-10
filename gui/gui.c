#include <gtk/gtk.h>
#include <string.h>

/* Global pointers to widgets */
GtkTextBuffer *text_buffer;
GtkWidget *entry;

/* Append message to log */
void append_log(const char *msg)
{
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    gtk_text_buffer_insert(text_buffer, &end, msg, -1);
    gtk_text_buffer_insert(text_buffer, &end, "\n", -1);
}

/* Send button callback */
void send_message(GtkButton *button, gpointer user_data)
{
    const char *msg = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(msg) == 0)
        return;

    append_log(msg);                          // Add typed message to log
    gtk_entry_set_text(GTK_ENTRY(entry), ""); // Clear input
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    /* --- Main window --- */
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Sample GTK GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* --- Layout --- */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* Scrollable text log */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    /* Input entry */
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    /* Send button */
    GtkWidget *send_btn = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(vbox), send_btn, FALSE, FALSE, 0);
    g_signal_connect(send_btn, "clicked", G_CALLBACK(send_message), NULL);

    /* Press Enter in entry to send message */
    g_signal_connect(entry, "activate", G_CALLBACK(send_message), NULL);

    /* Show everything */
    gtk_widget_show_all(window);

    /* Run GTK main loop */
    gtk_main();

    return 0;
}