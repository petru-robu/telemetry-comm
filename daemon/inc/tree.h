#ifndef TREE_H
#define TREE_H

#include "../../common/protocol.h"   // for tlm_msg

// Limits
#define MAX_CHILDREN 10
#define MAX_SUBSCRIBERS 20
#define CHANNEL_NAME_MAX 64

/* Channel node, keeps channel name, children channel nodes, clients subscribed to this channel (subscribers) */
typedef struct ChannelNode
{
    char name[CHANNEL_NAME_MAX];                
    struct ChannelNode *children[MAX_CHILDREN]; 
    int child_count;                            

    int subscribers[MAX_SUBSCRIBERS];  // socket fds of subscribers
    int sub_count;
} ChannelNode;


/* Root of the channel tree */
extern ChannelNode *root;

/* ---------------- Tree API ---------------- */

/* Initialize the channel tree */
void init_tree(void);

/**
 * Create a new channel node with the given name.
 * Returns a pointer to the new node or exits on failure.
 */
ChannelNode *create_node(const char *name);

/**
 * Find or create a channel by path.
 * Returns pointer to the channel node or NULL if failed.
 */
ChannelNode *get_or_create_channel(const char *path);

/**
 * Add a subscriber socket to the channel node.
 * Duplicate subs are ignored.
 */
void add_subscriber(ChannelNode *node, int fd);

/**
 * Remove a subscriber socket from all channels.
 */
void remove_subscriber(int fd);

/**
 * Broadcast a message recursively to this node and all child nodes.
 */
void broadcast_recursive(ChannelNode *node, struct tlm_msg *msg);

#endif // TREE_H
