
SUBDIRS  := $(wildcard labs/lab-*/)

all: $(SUBDIRS) 
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean: $(SUBDIRS) 
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

# make run-* => cd labs/lab-*/ + make run
run-%:
	$(MAKE) -C labs/lab-$* run

