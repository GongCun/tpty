DIRS = lib src
RUBY = $(shell which ruby 2>/dev/null)
ifneq (,$(RUBY))
	DIRS += vtparse
endif

all:
	for i in $(DIRS); do \
		(cd $$i && echo "making $$i" && $(MAKE)) || exit 1; \
	done

install:
	cd src && $(MAKE) install
	cd man && $(MAKE) install

clean:
	for i in $(DIRS) bin dbg test; do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done
