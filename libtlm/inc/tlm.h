#ifndef TLE_H
#define TLE_H

#include "../../common/protocol.h" // for message structure

typedef struct tlm_handle * tlm_t;

/* open a publisher / subscriber / both on a channel */
tlm_t tlm_open(int type, const char *channel_name);

/* register a real-time callback */
int tlm_callback(
    tlm_t token,
    void (*message_callback)(tlm_t token, const char *message)
);

/* read last received message (polling) */
const char * tlm_read(
    tlm_t token,
    unsigned int *message_id
);

/* post a message on the associated channel */
int tlm_post(tlm_t token, const char *message);

/* close the participant */
void tlm_close(tlm_t token);

/* return participant type */
int tlm_type(tlm_t token);



#endif