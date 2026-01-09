#ifndef SERVER_H
#define SERVER_H

#include "../../common/protocol.h" // for message structure
#include "tree.h" // for channel operations

// Max clients
#define MAX_CLIENTS 32

// Forward declarations
typedef struct client client_t;
typedef struct channel channel_t;

/* ---------------- Public Server API --------------- */
/* Initialize the server: client array, socket, and channel tree */
int server_init(const char *socket_path);

/* Run the main server loop: select() and handle events */
void server_run(void);

/* Cleanup the server socket and remove the socket file */
void server_cleanup(void);


#endif
