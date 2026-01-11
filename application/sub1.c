#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../../libtlm/inc/tlm.h"

void on_message(tlm_t handle, const char *msg)
{
    if(!handle)
        return;
    
    printf("[A] Received: %s\n", msg);

}

int main(void)
{
    tlm_t sub = tlm_open(TLM_SUBSCRIBER, "/comm");
    tlm_callback(sub, on_message);

    while(1)
        pause();

    tlm_close(sub);
    return 0;
}
