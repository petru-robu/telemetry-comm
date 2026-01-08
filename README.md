# Telemetry

Communication system in UNIX with hierarchical channels:

- An userspace daemon along with a library exposing its functionality, to provide short-message distribution among multiple participants.  

Messages are organized into hierarchical distribution channels: 
- A message sent to a parent channel is received by all subscribers of that channel and its child channels. 
- Parent channels are created automatically when a daemon registers the first subscriber on a child channel.  

There are three types of participants:  
- Publisher - send messages to a channel
- Subscriber - receive messages from channels they are subscribed to
- Publisher/Subscriber - can send and also receive messages


A possible set of channels at a given time could be:  

- A: `/comm/messaging/channel_a`  
- B: `/comm/messaging/channel_b`  
- C: `/comm/messaging`  
- D: `/comm`  

For this example, the following statements are correct:  

1. A message sent to channel **A** will be received only by **A**.  
2. A message sent to **C** will be received by **A** and **B**.  
3. A message sent to **D** will be received by **A**, **B**, **C**, and **D**.  
4. When channel **A** is created, the parent channels **C** and **D** will automatically become available.  


# Project Structure

The projects consists of three components:
- Userspace daemon
- API
- Client application

Each module is compiled separately via makefiles.

## Daemon
Octav docs here
### Channel Tree

### Server API

## API
api docs

## Client
client docs

# Running instructions

Compiling and building: 
```bash
# build all modules
make

# build daemon
make daemon 

# build client
make client

# etc
```

Running:
```bash
# run demon
./daemon/daemon

# run client
./client/client

```