#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include "../include/message.h"

int main() {
    int server_sock, client_sock;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len;
    struct tle_msg buffer;

    // Create Socket
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket error");
        exit(1);
    }

    // 2. Curatare adresa veche si setare
    unlink(SOCKET_PATH);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // 3. Bind & Listen
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        exit(1);
    }
    listen(server_sock, 10);
    printf("[DAEMON] Listening on %s...\n", SOCKET_PATH);

    // 4. Bucla simpla de acceptare (Iterativ pt inceput)
    while (1) {
        client_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);

        if (client_sock < 0) continue;

        printf("[DAEMON] Client connected!\n");

        // Citeste un mesaj de la client
        if (read(client_sock, &buffer, sizeof(buffer)) > 0) {
            if (buffer.type == MSG_SUBSCRIBE) {
                printf("[DAEMON] Subscribe request on channel: %s\n", buffer.channel);
            } else if (buffer.type == MSG_PUBLISH) {
                printf("[DAEMON] Publish request: %s on %s\n", buffer.payload, buffer.channel);
            }
        }

        close(client_sock); // Inchidem pt testul simplu
    }

    close(server_sock);
    unlink(SOCKET_PATH);
    return 0;
}