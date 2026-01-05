#ifndef TREE_H
#define TREE_H

#include "../include/message.h"

#define MAX_CHILDREN 10
#define MAX_SUBSCRIBERS 20

typedef struct ChannelNode {
    char name[64];
    struct ChannelNode *children[MAX_CHILDREN];
    int child_count;

    int subscribers[MAX_SUBSCRIBERS]; 
    int sub_count;
} ChannelNode;
extern ChannelNode *root;

ChannelNode* create_node(const char *name);
ChannelNode* get_or_create_channel(char *path);
void add_subscriber(ChannelNode *node, int fd);
void remove_subscriber(int fd); // Optional helper
void broadcast_recursive(ChannelNode *node, struct tle_msg *msg);
void init_tree();

#endif