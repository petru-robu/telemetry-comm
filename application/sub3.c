#include <stdio.h>
#include <stdlib.h>
#include "../../libtlm/inc/tlm.h"

void on_message(tlm_t t, const char *msg)
{
    printf("[C] Received: %s\n", msg);
}

int main(void)
{
    tlm_t sub = tlm_open(TLM_SUBSCRIBER, "/comm/messaging/channel");
    tlm_callback(sub, on_message);

    while(1)
        pause();

    tlm_close(sub);
    return 0;
}
