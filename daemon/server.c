#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/select.h>
#include "../include/message.h"
#include "tree.h"

int main() {
    int server_sock, max_fd, activity, new_socket;
    struct sockaddr_un server_addr;
    fd_set readfds;

    int client_sockets[30];
    for (int i = 0; i < 30; i++) client_sockets[i] = 0;

    // Setup Socket
    unlink(SOCKET_PATH);
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) { perror("Socket"); exit(1); }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind"); exit(1);
    }
    listen(server_sock, 10);
    printf("[DAEMON] Server listening on %s\n", SOCKET_PATH);

    // Initialize Tree
    init_tree();

    // 3. Event Loop
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        max_fd = server_sock;

        for (int i = 0; i < 30; i++) {
            int sd = client_sockets[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_fd) max_fd = sd;
        }

        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) printf("Select error\n");

        // New Connection
        if (FD_ISSET(server_sock, &readfds)) {
            new_socket = accept(server_sock, NULL, NULL);
            printf("[DAEMON] New connection, socket fd is %d\n", new_socket);

            for (int i = 0; i < 30; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // Handle IO
        for (int i = 0; i < 30; i++) {
            int sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds)) {
                struct tle_msg msg;
                int valread = read(sd, &msg, sizeof(msg));

                if (valread == 0) {
                    printf("[DAEMON] Host disconnected, fd %d\n", sd);
                    close(sd);
                    client_sockets[i] = 0;
                }
                else {
                    if (msg.type == MSG_SUBSCRIBE) {
                        printf("[CMD] Subscribe: %s on %s\n", (msg.client_flags == TLM_PUBLISHER ? "PUB" : "SUB"), msg.channel);
                        ChannelNode *node = get_or_create_channel(msg.channel);
                        if (msg.client_flags & TLM_SUBSCRIBER) {
                            add_subscriber(node, sd);
                        }
                    }
                    else if (msg.type == MSG_PUBLISH) {
                        printf("[CMD] Publish to %s: %s\n", msg.channel, msg.payload);
                        ChannelNode *node = get_or_create_channel(msg.channel);
                        broadcast_recursive(node, &msg);
                    }
                }
            }
        }
    }
    return 0;
}