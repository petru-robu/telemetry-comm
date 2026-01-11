#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tlm.h>

/* Global pointers to widgets */
GtkTextBuffer *pub_buffer;
GtkTextBuffer *sub_buffer;
GtkWidget *entry;

/* Global tlm handles */
tlm_t pub_handle = NULL;
tlm_t sub_handle = NULL;

/* Helper to append to a specific buffer */
void append_to_buffer(GtkTextBuffer *buffer, const char *msg)
{
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, msg, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
}

/* Structure to pass data to the main thread */
struct log_data {
    char *msg;
};

/* Idle callback to update Subscriber GUI from main thread */
gboolean update_sub_log(gpointer user_data)
{
    struct log_data *data = (struct log_data *)user_data;
    append_to_buffer(sub_buffer, data->msg);
    free(data->msg);
    free(data);
    return FALSE; // Remove from idle sources
}

/* Callback for subscriber */
void on_message(tlm_t t, const char *msg)
{
    (void)t;
    struct log_data *data = malloc(sizeof(struct log_data));
    if (data) {
        size_t len = strlen(msg) + 20;
        data->msg = malloc(len);
        if (data->msg) {
            snprintf(data->msg, len, "[SUB] Received: %s", msg);
            g_idle_add(update_sub_log, data);
        } else {
            free(data);
        }
    }
}

/* Send button callback */
void send_message(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;
    const char *msg = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(msg) == 0)
        return;

    // Publish the message
    if (pub_handle) {
        tlm_post(pub_handle, msg);
        
        // Log sent message to Publisher tab
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "[PUB] Sent: %s", msg);
        append_to_buffer(pub_buffer, log_msg);
    } else {
        append_to_buffer(pub_buffer, "Error: Publisher not initialized");
    }

    gtk_entry_set_text(GTK_ENTRY(entry), ""); // Clear input
}

GtkWidget* create_publisher_tab() {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    /* Scrollable text log */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    pub_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    /* Input entry */
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    /* Send button */
    GtkWidget *send_btn = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(vbox), send_btn, FALSE, FALSE, 0);
    g_signal_connect(send_btn, "clicked", G_CALLBACK(send_message), NULL);
    g_signal_connect(entry, "activate", G_CALLBACK(send_message), NULL);

    return vbox;
}

GtkWidget* create_subscriber_tab() {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    /* Scrollable text log */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    sub_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    return vbox;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    /* --- Initialize TLM --- */
    pub_handle = tlm_open(TLM_PUBLISHER, "/comm");
    if (!pub_handle) {
        fprintf(stderr, "Failed to open publisher\n");
    }

    sub_handle = tlm_open(TLM_SUBSCRIBER, "/comm/A/B");
    if (sub_handle) {
        tlm_callback(sub_handle, on_message);
    } else {
        fprintf(stderr, "Failed to open subscriber\n");
    }

    /* --- Main window --- */
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "TLM GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* --- Notebook (Tabs) --- */
    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    /* Tab 1: Publisher */
    GtkWidget *pub_tab = create_publisher_tab();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), pub_tab, gtk_label_new("Publisher"));

    /* Tab 2: Subscriber */
    GtkWidget *sub_tab = create_subscriber_tab();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sub_tab, gtk_label_new("Subscriber"));

    /* Show everything */
    gtk_widget_show_all(window);

    /* Run GTK main loop */
    gtk_main();

    /* Cleanup */
    if (pub_handle) tlm_close(pub_handle);
    if (sub_handle) tlm_close(sub_handle);

    return 0;
}