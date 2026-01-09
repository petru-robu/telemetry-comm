#include <stdio.h>
#include <unistd.h>
#include "../../libtlm/inc/tlm.h"

int main(void)
{
    tlm_t pub = tlm_open(TLM_PUBLISHER, "/comm");

    for (int i = 1; i <= 5; i++)
    {
        char msg[64];
        snprintf(msg, sizeof(msg), "Hello #%d", i);
        tlm_post(pub, msg);
        printf("Sent: %s\n", msg);
        sleep(1);
    }

    tlm_close(pub);
    return 0;
}
