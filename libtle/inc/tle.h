#ifndef TLE_H
#define TLE_H

#define TLM_PUBLISHER 0x1
#define TLM_SUBSCRIBER 0x2
#define TLM_BOTH 0x3

// example api functions
// implement in tle.c

// inregistrarea unui participant
void tlm_open(
    int type, //publisher, subscriber pr both
    const char* channel_name
);

// inregistrarea unei functii "callback" (pentru notificare in timp real):
int tlm_callback(
);

// obtinerea ultimului mesaj primit
const char* tlm_read(
);

// postarea unui mesaj pe un canal
int tlm_post(
);

// deinregistrarea participantilor
void tlm_close(
);



#endif