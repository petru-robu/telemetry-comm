#include "../../common/inc/message.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/pubsub_daemon.sock"

int main(void)
{
    int sock;
    struct sockaddr_un addr;

    // 1. Create socket
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 2. Connect to server
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("[CLIENT] Connected to server\n");

    // 3. Send a subscribe message
    struct tle_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_SUBSCRIBE;
    msg.client_flags = TLM_SUBSCRIBER;
    strncpy(msg.channel, "test/channel", sizeof(msg.channel) - 1);

    if (write(sock, &msg, sizeof(msg)) < 0)
    {
        perror("write");
    }
    else
    {
        printf("[CLIENT] Subscribed to %s\n", msg.channel);
    }

    // 4. Read messages from server
    while (1)
    {
        struct tle_msg incoming;
        int n = read(sock, &incoming, sizeof(incoming));
        if (n <= 0)
        {
            printf("[CLIENT] Server closed connection\n");
            break;
        }

        if (incoming.type == MSG_DELIVER)
        {
            printf("[CLIENT] Message on %s: %s\n",
                   incoming.channel, incoming.payload);
        }
    }

    close(sock);
    return 0;
}
