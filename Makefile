# Builds every module

.PHONY: all common libtle daemon client clean

all: common libtle daemon client

common:
	$(MAKE) -C common

libtle:
	$(MAKE) -C libtle

daemon: common libtle
	$(MAKE) -C daemon

client: common libtle
	$(MAKE) -C client

clean:
	$(MAKE) -C common clean
	$(MAKE) -C libtle clean
	$(MAKE) -C daemon clean
	$(MAKE) -C client clean
