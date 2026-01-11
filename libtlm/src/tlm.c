#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "../inc/tlm.h"
#include "../../common/protocol.h"

// internal tlm connection handle
struct tlm_handle 
{
    int sock;
    int type;
    char channel[MAX_CHANNEL];

    // callback 
    void (*callback)(tlm_t, const char *);
    pthread_t rx_thread;
    int rx_running;

    // polling
    char last_msg[MAX_MSG];
    unsigned int msg_id;
    int has_msg;
};

// connect to the daemon
static int connect_to_daemon(void)
{
    // create unix socket
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;

    // create socket address structure
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // try connecting
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(sock);
        return -1;
    }

    return sock;
}

// loopback
static void *rx_loop(void *arg)
{
    tlm_t handle = arg; // connection handle from arg
    struct tlm_msg msg;

    // try reading
    while (handle->rx_running)
    {   
        int r = read(handle->sock, &msg, sizeof(msg));
        if (r <= 0)
            break;

        strncpy(handle->last_msg, msg.payload, MAX_MSG - 1);
        handle->last_msg[MAX_MSG - 1] = '\0';

        handle->msg_id++;
        handle->has_msg = 1;

        if (handle->callback) // call the callback function
            handle->callback(handle, msg.payload);
    }

    return NULL;
}

/* ------------- PUBLIC API ---------------*/
tlm_t tlm_open(int type, const char *channel_name)
{   
    // new handle for the new connection 
    tlm_t handle = calloc(1, sizeof(*handle));

    if (!handle)
        return NULL;

    // try connecting to the daemon
    handle->sock = connect_to_daemon();
    if (handle->sock < 0)
    {
        free(handle);
        return NULL;
    }
    
    handle->type = type;
    strncpy(handle->channel, channel_name, MAX_CHANNEL - 1);

    struct tlm_msg msg = {
        .type = MSG_SUBSCRIBE,
        .client_flags = type
    };
    strncpy(msg.channel, channel_name, MAX_CHANNEL - 1);

    
    // try writing subscrition message
    if (write(handle->sock, &msg, sizeof(msg)) <= 0)
    {
        close(handle->sock);
        free(handle);
        return NULL;
    }

    return handle;
}

// post a message to a channel
int tlm_post(tlm_t handle, const char *message)
{   
    // for publisher only
    if (!handle || !(handle->type & TLM_PUBLISHER))
        return -1;

    // prepare message
    struct tlm_msg msg = {
        .type = MSG_PUBLISH,
        .client_flags = handle->type
    };
    strncpy(msg.channel, handle->channel, MAX_CHANNEL- 1);
    strncpy(msg.payload, message, MAX_MSG - 1);


    // try writing on the socket
    if(write(handle->sock, &msg, sizeof(msg)) > 0)
        return 0;
    else
        return -1;
}

// register a callback on a connection
int tlm_callback(tlm_t handle, void (*message_callback)(tlm_t, const char *))
{
    // no handle or no callback function given
    if (!handle || !message_callback)
        return -1;

    handle->callback = message_callback;
    handle->rx_running = 1;

    // create a new thread and run from rx_loop
    return pthread_create(&handle->rx_thread, NULL, rx_loop, handle);
}

// get last message sent
const char * tlm_read(tlm_t handle, unsigned int *message_id)
{
    if (!handle || !handle->has_msg)
        return NULL;

    // get the last message id
    handle->has_msg = 0;
    if (message_id)
        *message_id = handle->msg_id;

    return handle->last_msg;
}

// close connection
void tlm_close(tlm_t handle)
{
    if (!handle)
        return;

    // stop the thread for callback
    handle->rx_running = 0;
    if (handle->callback)
        pthread_join(handle->rx_thread, NULL);
    close(handle->sock);
    free(handle);
}

int tlm_type(tlm_t handle)
{
    // return handle type
    if(handle)
        return handle->type;
    else
        return 0;
}