#ifndef PROTOCOL_H
#define PROTOCOL_H

// Message limits
#define MAX_CHANNEL 128
#define MAX_MSG 256

// Client types
#define TLM_PUBLISHER 0x1
#define TLM_SUBSCRIBER 0x2
#define TLM_BOTH 0x3

enum msg_type
{
    MSG_SUBSCRIBE,
    MSG_UNSUBSCRIBE,
    MSG_PUBLISH,
    MSG_DELIVER
};

struct tle_msg
{
    enum msg_type type;
    int client_flags;
    char channel[MAX_CHANNEL];
    char payload[MAX_MSG];
};

#endif