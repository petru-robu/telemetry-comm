#include "../inc/server.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

/* ---------- Module-level state ---------- */

/* Server socket file descriptor */
static int server_sock;

/* UNIX socket address for the server */
static struct sockaddr_un server_addr;

/* Array of connected client sockets. 0 means slot is free */
static int client_sockets[MAX_CLIENTS];

/* ---------- Internal (static) helpers ---------- */

/* Initialize the client sockets array to 0 */
static void init_client_sockets(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_sockets[i] = 0;
}

/* Setup the server socket: create, bind, and listen */
static int setup_socket(const char *socket_path)
{
    /* Remove any existing socket file to avoid bind errors */
    unlink(socket_path);

    /* Create a UNIX socket */
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("socket");
        return -1;
    }

    /* Initialize the socket address structure */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path,
            sizeof(server_addr.sun_path) - 1);

    /* Bind the socket to the specified path */
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_sock);
        return -1;
    }

    /* Start listening for incoming connections */
    if (listen(server_sock, 10) < 0)
    {
        perror("listen");
        close(server_sock);
        return -1;
    }

    printf("[DAEMON] Server listening on %s\n", socket_path);
    return 0;
}

/* Accept a new client connection and add it to the client_sockets array */
static void handle_new_connection(void)
{
    int new_socket = accept(server_sock, NULL, NULL);
    if (new_socket < 0)
    {
        perror("accept");
        return;
    }

    printf("[DAEMON] New connection, fd %d\n", new_socket);

    /* Find the first free slot to store this client */
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] == 0)
        {
            client_sockets[i] = new_socket;
            return;
        }
    }

    /* If we reach here, server is full; refuse the connection */
    close(new_socket);
}

/* Handle an incoming message from a client at client_sockets[index] */
static void handle_client_message(int index)
{
    int sd = client_sockets[index];
    struct tlm_msg msg;

    int valread = read(sd, &msg, sizeof(msg));

    if (valread == 0)
    {
        /* Client disconnected */
        printf("[DAEMON] Client disconnected, fd %d\n", sd);
        close(sd);
        client_sockets[index] = 0;
        return;
    }

    if (msg.type == MSG_SUBSCRIBE)
    {
        /* Subscribe this client to a channel */
        printf("[CMD] Subscribe: %s on %s\n",
               (msg.client_flags & TLM_PUBLISHER) ? "PUB" : "SUB",
               msg.channel);

        ChannelNode *node = get_or_create_channel(msg.channel);

        if (msg.client_flags & TLM_SUBSCRIBER)
            add_subscriber(node, sd);
    }
    else if (msg.type == MSG_PUBLISH)
    {
        /* Publish a message to all subscribers */
        printf("[CMD] Publish to %s: %s\n",
               msg.channel, msg.payload);

        ChannelNode *node = get_or_create_channel(msg.channel);
        broadcast_recursive(node, &msg);
    }
}

/* ---------- Public Server API ---------- */

/* Initialize the server: client array, socket, and channel tree */
int server_init(const char *socket_path)
{
    init_client_sockets();

    if (setup_socket(socket_path) < 0)
        return -1;

    /* Initialize the message channel tree */
    init_tree();

    return 0;
}

/* Run the main server loop: select() and handle events */
void server_run(void)
{
    fd_set readfds;

    while (1)
    {
        /* Clear the fd set and add the server socket */
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        int max_fd = server_sock;

        /* Add all connected clients to the fd set */
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_sockets[i];
            if (sd > 0)
                FD_SET(sd, &readfds);

            if (sd > max_fd)
                max_fd = sd;
        }

        /* Wait for activity on any socket */
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR)
        {
            perror("select");
            continue;
        }

        /* Handle a new incoming connection */
        if (FD_ISSET(server_sock, &readfds))
            handle_new_connection();

        /* Handle incoming messages from clients */
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] > 0 &&
                FD_ISSET(client_sockets[i], &readfds))
            {
                handle_client_message(i);
            }
        }
    }
}

/* Cleanup the server socket and remove the socket file */
void server_cleanup(void)
{
    close(server_sock);
    unlink(server_addr.sun_path);
}
