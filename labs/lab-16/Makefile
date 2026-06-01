SUBDIRS := part-1 part-2 part-3 part-4

all: $(SUBDIRS)
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean: $(SUBDIRS)
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

run-1-server:
	$(MAKE) -C part-1 run-1-server

run-1-client:
	$(MAKE) -C part-1 run-1-client

run-2-server:
	$(MAKE) -C part-1 run-2-server

run-2-client:
	$(MAKE) -C part-1 run-2-client

run-3-server:
	$(MAKE) -C part-1 run-3-server

run-3-client:
	$(MAKE) -C part-1 run-3-client

run-4-server:
	$(MAKE) -C part-1 run-4-server

run-4-client:
	$(MAKE) -C part-1 run-4-client

run-2-1-server:
	$(MAKE) -C part-2 run-1-server

run-2-1-client:
	$(MAKE) -C part-2 run-1-client

run-2-2-server:
	$(MAKE) -C part-2 run-2-server

run-2-2-client:
	$(MAKE) -C part-2 run-2-client

run-2-3-server:
	$(MAKE) -C part-2 run-3-server

run-2-3-client:
	$(MAKE) -C part-2 run-3-client

run-2-4-server:
	$(MAKE) -C part-2 run-4-server

run-2-4-client:
	$(MAKE) -C part-2 run-4-client

run-2-4-tcp-client:
	$(MAKE) -C part-2 run-4-tcp-client

run-2-4-udp-client:
	$(MAKE) -C part-2 run-4-udp-client

run-3-1-server:
	$(MAKE) -C part-3 run-1-server

run-3-1-client:
	$(MAKE) -C part-3 run-1-client

run-3-2-server:
	$(MAKE) -C part-3 run-2-server

run-3-2-client:
	$(MAKE) -C part-3 run-2-client

run-4-server-raw:
	$(MAKE) -C part-4 run-server

run-4-1-client:
	$(MAKE) -C part-4 run-1-client

run-4-2-client:
	$(MAKE) -C part-4 run-2-client

run-4-3-client:
	$(MAKE) -C part-4 run-3-client

