#include <stdio.h>
#include "../../libtlm/inc/tlm.h"

void on_message(tlm_t t, const char *msg)
{
    printf("Received: %s\n", msg);
}

int main(void)
{
    tlm_t sub = tlm_open(TLM_SUBSCRIBER, "/comm/messaging");

    tlm_callback(sub, on_message);

    while (1)
        pause();

    tlm_close(sub);
    return 0;
}
