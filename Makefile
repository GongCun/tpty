DIRS = lib src

all:
	for i in $(DIRS); do \
		(cd $$i && echo "making $$i" && $(MAKE)) || exit 1; \
	done

install:
	cd src && $(MAKE) install

clean:
	for i in $(DIRS) bin dbg vtparse; do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done
