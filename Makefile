# Builds every module

.PHONY: all libtlm daemon application clean

all: libtlm daemon application

libtlm:
	$(MAKE) -C libtlm

daemon:
	$(MAKE) -C daemon

application:
	$(MAKE) -C application
	
clean:
	$(MAKE) -C libtlm clean
	$(MAKE) -C daemon clean
	$(MAKE) -C application clean
