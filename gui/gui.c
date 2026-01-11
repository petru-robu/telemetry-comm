#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tlm.h>

#define MAX_TABS 32

/* Structure to hold tab information */
typedef struct {
    tlm_t handle;
    GtkTextBuffer *buffer;
    GtkWidget *entry; // For input in publisher tabs
    char path[256];
    int is_subscriber;
} TabInfo;

/* Global state */
TabInfo tabs[MAX_TABS];
int tab_count = 0;
GtkWidget *notebook;
GtkWidget *path_entry; // In the control tab

/* Structure to pass data from background thread to GUI thread */
struct log_data {
    TabInfo *tab;
    char *msg;
};

/* Helper to append to a specific buffer */
void append_to_buffer(GtkTextBuffer *buffer, const char *msg)
{
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, msg, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);
}

/* Idle callback to update Subscriber GUI from main thread */
gboolean update_sub_log(gpointer user_data)
{
    struct log_data *data = (struct log_data *)user_data;
    if (data->tab && data->tab->buffer) {
        append_to_buffer(data->tab->buffer, data->msg);
    }
    free(data->msg);
    free(data);
    return FALSE; // Remove from idle sources
}

/* Callback for subscriber (called from libtlm thread) */
void on_message(tlm_t t, const char *msg)
{
    // Find the tab associated with this handle
    TabInfo *found_tab = NULL;
    for (int i = 0; i < tab_count; i++) {
        if (tabs[i].handle == t) {
            found_tab = &tabs[i];
            break;
        }
    }

    if (!found_tab) return;

    struct log_data *data = malloc(sizeof(struct log_data));
    if (data) {
        size_t len = strlen(msg) + 32;
        data->msg = malloc(len);
        if (data->msg) {
            snprintf(data->msg, len, "[SUB] Received: %s", msg);
            data->tab = found_tab;
            g_idle_add(update_sub_log, data);
        } else {
            free(data);
        }
    }
}

/* Send button callback for Publisher tabs */
void send_message(GtkButton *button, gpointer user_data)
{
    (void)button;
    TabInfo *tab = (TabInfo *)user_data;
    
    const char *msg = gtk_entry_get_text(GTK_ENTRY(tab->entry));
    if (strlen(msg) == 0)
        return;

    // Publish the message
    if (tab->handle) {
        tlm_post(tab->handle, msg);
        
        // Log sent message
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "[PUB] Sent: %s", msg);
        append_to_buffer(tab->buffer, log_msg);
    } else {
        append_to_buffer(tab->buffer, "Error: Handle not valid");
    }

    gtk_entry_set_text(GTK_ENTRY(tab->entry), ""); // Clear input
}

/* Create a new tab content widget */
GtkWidget* create_tab_content(TabInfo *tab) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    /* Scrollable text log */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    tab->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    if (!tab->is_subscriber) {
        /* Input entry for Publisher */
        tab->entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX(vbox), tab->entry, FALSE, FALSE, 0);

        /* Send button */
        GtkWidget *send_btn = gtk_button_new_with_label("Send");
        gtk_box_pack_start(GTK_BOX(vbox), send_btn, FALSE, FALSE, 0);
        g_signal_connect(send_btn, "clicked", G_CALLBACK(send_message), tab);
        g_signal_connect(tab->entry, "activate", G_CALLBACK(send_message), tab);
    }

    return vbox;
}

/* Add a new tab */
void add_tab(const char *path, int is_subscriber) {
    if (tab_count >= MAX_TABS) {
        fprintf(stderr, "Max tabs reached\n");
        return;
    }

    TabInfo *tab = &tabs[tab_count];
    strncpy(tab->path, path, sizeof(tab->path) - 1);
    tab->is_subscriber = is_subscriber;

    // Open TLM handle
    int type = is_subscriber ? TLM_SUBSCRIBER : TLM_PUBLISHER;
    tab->handle = tlm_open(type, path);
    
    if (!tab->handle) {
        fprintf(stderr, "Failed to open %s on %s\n", is_subscriber ? "SUB" : "PUB", path);
        return;
    }

    if (is_subscriber) {
        tlm_callback(tab->handle, on_message);
    }

    // Create GUI elements
    GtkWidget *content = create_tab_content(tab);
    
    char label_text[300];
    snprintf(label_text, sizeof(label_text), "%s: %s", is_subscriber ? "SUB" : "PUB", path);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), content, gtk_label_new(label_text));
    gtk_widget_show_all(content);
    
    // Switch to new tab
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), -1);

    tab_count++;
}

/* Callbacks for Control Panel */
void on_add_pub_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    (void)user_data;
    const char *path = gtk_entry_get_text(GTK_ENTRY(path_entry));
    if (strlen(path) > 0) {
        add_tab(path, 0); // 0 = Publisher
    }
}

void on_add_sub_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    (void)user_data;
    const char *path = gtk_entry_get_text(GTK_ENTRY(path_entry));
    if (strlen(path) > 0) {
        add_tab(path, 1); // 1 = Subscriber
    }
}

/* Create the Control Panel tab */
GtkWidget* create_control_panel() {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    GtkWidget *label = gtk_label_new("Create New Connection");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    path_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(path_entry), "/comm/channel");
    gtk_box_pack_start(GTK_BOX(vbox), path_entry, FALSE, FALSE, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *btn_pub = gtk_button_new_with_label("Add Publisher");
    g_signal_connect(btn_pub, "clicked", G_CALLBACK(on_add_pub_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), btn_pub, TRUE, TRUE, 0);

    GtkWidget *btn_sub = gtk_button_new_with_label("Add Subscriber");
    g_signal_connect(btn_sub, "clicked", G_CALLBACK(on_add_sub_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), btn_sub, TRUE, TRUE, 0);

    return vbox;
}

void cleanup_tlm(void) {
    for (int i = 0; i < tab_count; i++) {
        if (tabs[i].handle) {
            tlm_close(tabs[i].handle);
            tabs[i].handle = NULL;
        }
    }
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    /* --- Main window --- */
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "TLM GUI Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* --- Notebook --- */
    notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    /* Tab 1: Control Panel */
    GtkWidget *control_tab = create_control_panel();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), control_tab, gtk_label_new("Menu"));

    /* Show everything */
    gtk_widget_show_all(window);

    /* Run GTK main loop */
    gtk_main();

    /* Cleanup */
    cleanup_tlm();

    return 0;
}