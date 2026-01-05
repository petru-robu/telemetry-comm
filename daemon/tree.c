#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tree.h"

ChannelNode *root = NULL;

void init_tree() {
    root = create_node("/");
}

ChannelNode* create_node(const char *name) {
    ChannelNode *node = malloc(sizeof(ChannelNode));
    if (!node) { perror("Malloc failed"); exit(1); }
    
    strncpy(node->name, name, 63);
    node->child_count = 0;
    node->sub_count = 0;
    
    for(int i=0; i<MAX_CHILDREN; i++) node->children[i] = NULL;
    for(int i=0; i<MAX_SUBSCRIBERS; i++) node->subscribers[i] = -1;
    
    return node;
}

ChannelNode* find_child(ChannelNode *parent, const char *name) {
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) {
            return parent->children[i];
        }
    }
    return NULL;
}

ChannelNode* get_or_create_channel(char *path) {
    if (!root) init_tree();

    ChannelNode *current = root;
    char *token_str = strdup(path); 
    char *token = strtok(token_str, "/");

    while (token != NULL) {
        ChannelNode *next = find_child(current, token);
        
        if (next == NULL) {
            if (current->child_count >= MAX_CHILDREN) {
                printf("[ERROR] Tree full at node %s\n", current->name);
                free(token_str);
                return NULL;
            }
            next = create_node(token);
            current->children[current->child_count++] = next;
            printf("[TREE] Created new channel segment: %s\n", token);
        }
        
        current = next;
        token = strtok(NULL, "/");
    }
    
    free(token_str);
    return current;
}

void add_subscriber(ChannelNode *node, int fd) {
    if (!node) return;
    for (int i = 0; i < node->sub_count; i++) {
        if (node->subscribers[i] == fd) return;
    }
    if (node->sub_count < MAX_SUBSCRIBERS) {
        node->subscribers[node->sub_count++] = fd;
        printf("[TREE] Socket %d subscribed to %s\n", fd, node->name);
    }
    else
        printf("[ERROR] Too many Subscribers");
}

void broadcast_recursive(ChannelNode *node, struct tle_msg *msg) {
    if (!node) return;

    for (int i = 0; i < node->sub_count; i++) {
        int client_fd = node->subscribers[i];
        
        // Prepare message
        struct tle_msg out_msg = *msg; 
        out_msg.type = MSG_DELIVER; 
        
        // Write to socket
        if (write(client_fd, &out_msg, sizeof(out_msg)) < 0) {
            printf("[ERROR] Writing to socket]");
        }
    }

    for (int i = 0; i < node->child_count; i++) {
        broadcast_recursive(node->children[i], msg);
    }
}