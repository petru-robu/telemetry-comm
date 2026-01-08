#include "../inc/server.h"
#include <stdlib.h>

int main(void)
{
    if (server_init(SOCKET_PATH) < 0)
        return EXIT_FAILURE;

    server_run();
    server_cleanup();

    return EXIT_SUCCESS;
}
