#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

// Maxim sizes
#define SOCKET_PATH "/tmp/pubsub_daemon.sock"
#define MAX_CLIENTS 32
#define MAX_CHANNEL 128
#define MAX_MSG 256

// Forward declarations
struct client;
struct channel;

// Message types
typedef enum
{
    MSG_SUBSCRIBE,
    MSG_UNSUBSCRIBE,
    MSG_PUBLISH,
    MSG_DELIVER
} msg_type_t;

// Generic message structure
typedef struct
{
    msg_type_t type;
    char channel[MAX_CHANNEL];
    char payload[MAX_MSG];
} ps_msg_t;

// Subscriber / client structure
typedef struct client
{
    int fd;           // socket file descriptor
    pthread_t thread; // optional thread for handling client
    struct client *next;
} client_t;

// Channel structure (tree)
typedef struct channel
{
    char name[MAX_CHANNEL];
    struct channel *parent;
    struct channel **children;
    size_t nchildren;
    client_t *subscribers; // linked list of subscribers
} channel_t;

// Server functions
int server_init(const char *socket_path);
void server_run(void);
void server_shutdown(void);
int send_message(int client_fd, ps_msg_t *msg);
int broadcast_message(channel_t *ch, ps_msg_t *msg);

#endif // SERVER_H
