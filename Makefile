# Builds every module

.PHONY: all libtlm daemon application clean

all: libtlm daemon application gui

libtlm:
	$(MAKE) -C libtlm

daemon:
	$(MAKE) -C daemon

application:
	$(MAKE) -C application
	
gui:
	$(MAKE) -C gui

clean:
	$(MAKE) -C libtlm clean
	$(MAKE) -C daemon clean
	$(MAKE) -C application clean
