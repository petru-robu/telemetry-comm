#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tree.h"

ChannelNode *root = NULL;

void init_tree(void)
{
    if (!root)
        root = create_node("/"); // Root channel
}

ChannelNode *create_node(const char *name)
{
    ChannelNode *node = malloc(sizeof(ChannelNode));
    if (!node)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    strncpy(node->name, name, CHANNEL_NAME_MAX - 1);
    node->name[CHANNEL_NAME_MAX - 1] = '\0'; // ensure null-termination

    node->child_count = 0;
    node->sub_count = 0;

    for (int i = 0; i < MAX_CHILDREN; i++)
        node->children[i] = NULL;

    for (int i = 0; i < MAX_SUBSCRIBERS; i++)
        node->subscribers[i] = -1;

    return node;
}

/* ---------- Helpers ---------- */

/* Find a child node by name under parent. Returns NULL if not found */
static ChannelNode *find_child(ChannelNode *parent, const char *name)
{
    if (!parent || !name)
        return NULL;

    for (int i = 0; i < parent->child_count; i++)
    {
        if (strcmp(parent->children[i]->name, name) == 0)
            return parent->children[i];
    }
    return NULL;
}

/* ---------- Tree operations ---------- */

ChannelNode *get_or_create_channel(const char *path)
{
    if (!root)
        init_tree();

    if (!path || path[0] == '\0') // empty path
        return root;

    ChannelNode *current = root;
    char *token_str = strdup(path);
    if (!token_str)
    {
        perror("strdup failed");
        return NULL;
    }

    char *token = strtok(token_str, "/");
    while (token != NULL)
    {
        ChannelNode *next = find_child(current, token);

        if (!next)
        {
            if (current->child_count >= MAX_CHILDREN)
            {
                fprintf(stderr, "[ERROR] Node %s full, cannot add %s\n", current->name, token);
                free(token_str);
                return NULL;
            }

            next = create_node(token);
            current->children[current->child_count++] = next;
            printf("[TREE] Created new channel: %s\n", token);
        }

        current = next;
        token = strtok(NULL, "/");
    }

    free(token_str);
    return current;
}

/* Debug: Print tree recursively */
void print_tree_recursive(ChannelNode *node, int depth)
{
    if (!node) return;

    for (int i = 0; i < depth; i++) printf("  ");
    printf("- %s (Subs: ", node->name);
    for (int i = 0; i < node->sub_count; i++)
        printf("%d ", node->subscribers[i]);
    printf(")\n");

    for (int i = 0; i < node->child_count; i++)
        print_tree_recursive(node->children[i], depth + 1);
}

void print_tree(void)
{
    if (root)
    {
        printf("--- Tree Structure ---\n");
        print_tree_recursive(root, 0);
        printf("----------------------\n");
    }
}

void add_subscriber(ChannelNode *node, int fd)
{
    if (!node || fd < 0)
        return;

    // Avoid duplicate subscriptions
    for (int i = 0; i < node->sub_count; i++)
        if (node->subscribers[i] == fd)
            return;

    if (node->sub_count < MAX_SUBSCRIBERS)
    {
        node->subscribers[node->sub_count++] = fd;
        printf("[TREE] Socket %d subscribed to %s\n", fd, node->name);
        // print_tree(); // Print tree after adding
    }
    else
    {
        fprintf(stderr, "[ERROR] Too many subscribers for channel %s\n", node->name);
    }
}

/* Remove a subscriber from a channel (node) and recursively remove him from children nodes */
void remove_from_node(ChannelNode *node, int sub_fd)
{
    for (int i = 0; i < node->sub_count; i++)
    {
        if (node->subscribers[i] == sub_fd)
        {
            // Shift remaining subscribers left
            for (int j = i; j < node->sub_count - 1; j++)
                node->subscribers[j] = node->subscribers[j + 1];

            node->sub_count--;
            break;
        }
    }

    for (int i = 0; i < node->child_count; i++)
        remove_from_node(node->children[i], sub_fd);
}


void remove_subscriber(int fd)
{
    if (!root || fd < 0)
        return;

    remove_from_node(root, fd);
    printf("[TREE] Removed subscriber %d\n", fd);
    // print_tree(); // Print tree after removing
}

/* Recursively broadcast a message to all subscribers of this node and its children */
void broadcast_recursive(ChannelNode *node, struct tlm_msg *msg)
{
    if (!node || !msg)
        return;

    for (int i = 0; i < node->sub_count; i++)
    {
        int client_fd = node->subscribers[i];
        if (client_fd < 0)
            continue;

        struct tlm_msg out_msg = *msg;
        out_msg.type = MSG_DELIVER;

        if (write(client_fd, &out_msg, sizeof(out_msg)) < 0)
            perror("[ERROR] writing to socket");
    }

    // Recurse to children
    for (int i = 0; i < node->child_count; i++)
        broadcast_recursive(node->children[i], msg);
}
