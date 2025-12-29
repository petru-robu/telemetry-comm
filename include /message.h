#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_CHANNEL 128
#define MAX_MSG     256

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
    char payload[MAX_MSG];
};

#endif