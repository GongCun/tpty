DIRS = lib src
RUBY = $(shell type ruby 2>&1)
ifeq (,$(findstring not found,$(RUBY)))
	DIRS += vtparse
endif

all:
	for i in $(DIRS); do \
		(cd $$i && echo "making $$i" && $(MAKE)) || exit 1; \
	done

install:
	cd src && $(MAKE) install
	cd man && ($(MAKE) -B -k && $(MAKE) install)

clean:
	for i in $(DIRS) bin dbg test; do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done
