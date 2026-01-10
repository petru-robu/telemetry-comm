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
- daemon (Server)
- libtlm (API that exposes the server's functionality)
- application (links libtlm and communicates with the daemon)

Each module is compiled separately via makefiles.

## Daemon
Octav docs here
### Channel Tree

### Server API
docs here


# Running instructions

Compiling and building: 
```bash
# build all modules
make

# build daemon
make daemon 

# etc
```

Running:
```bash
# run demon
./daemon/daemon

```

Deci asa 3 chestii:
- daemon = server care ruleaza tot timpul
- libtlm = api ca sa interactionezi cu serverul, deci un fel de client sa zic asa
- application = aplicatii care linkeaza libtlm si fac chestii cu ele

Si in application pub1.c, pub2.c, sub1.c, etc astia sunt clienti practic.

Si ce trebuie sa faci este sa te uiti cum dau publish/ subscribe in pub1.c sub1.c si asa si sa
bagi asta in gui. Deocamdata in gui e gol nu e inclusa libraria. 

Ca sa rulezi folosesti makefilurile alea. Dai cd in folderul respectiv si dai make acolo si dupa rulezi.

Cand linkezi libtlm in gui tre sa modifici si in makefile. Mai vezi si la celelalte makefile uri cum am facut. Practic 
o sa iti mai modifici makefile-ul.

Sa ai gui/sub.c, gui/pub.c astea sa fie gui-uri diferite sau ceva.