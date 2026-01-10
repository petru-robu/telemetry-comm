#ifndef TLE_H
#define TLE_H

#include "../../common/protocol.h" // for message structure

typedef struct tlm_handle* tlm_t;

// subscribe a participant to a channel (open a handle)
tlm_t tlm_open(int type, const char *channel_name);

// register a callback 
int tlm_callback(tlm_t handle, void (*message_callback)(tlm_t handle, const char *message));

// read last received message for polling applications
const char * tlm_read(tlm_t handle, unsigned int *message_id);

// post a message on the channel the connection is subscribed to
int tlm_post(tlm_t handle, const char *message);

// close the connection
void tlm_close(tlm_t token);

// return the type of connection (publisher / subscriber / both)
int tlm_type(tlm_t token);



#endif