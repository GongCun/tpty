ROOT=..
PLATFORM=$(shell $(ROOT)/systype.sh | tr 'a-z' 'A-Z')
EXTRALIBS=$(shell $(ROOT)/config.sh)
ifneq (,$(findstring xlc,$(shell which $(CC) 2>/dev/null)))
include $(ROOT)/Make.defines.xlc
else
include $(ROOT)/Make.defines.gcc
endif
CFLAGS += -D$(PLATFORM)
INSTALL_PATH = /usr/local/bin

ifneq (,$(findstring -lcrypto,$(EXTRALIBS)))
	PROGS =	tpty tptyreplay tpty-crypt
else
	PROGS =	tpty tptyreplay
endif

all:	$(PROGS)

tpty:	main.o loop.o do_driver.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LDFLAGS) $(LDLIBS)
	@[ -d ../bin/ ] || mkdir -p ../bin
	@(cd ../dbg; ln -sf ../bin/$@ .)

tptyreplay: tptyreplay.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LDFLAGS) $(LDLIBS)
	@[ -d ../bin/ ] || mkdir -p ../bin
	@cp -p $@ ../bin
	@(cd ../dbg; ln -sf ../bin/$@ .)

tpty-crypt: tpty-crypt.o
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LDFLAGS) $(LDLIBS)
	@[ -d ../bin/ ] || mkdir -p ../bin
	@cp -p $@ ../bin
	@(cd ../dbg; ln -sf ../bin/$@ .)

install: $(PROGS)
	@[ -d $(INSTALL_PATH)/ ] || \
		(mkdir -p $(INSTALL_PATH)/; chmod 755 $(INSTALL_PATH)/)
	@for prog in ${PROGS}; do \
		echo "copy $$prog to ${INSTALL_PATH}" && \
	       	install -s -S -f $(INSTALL_PATH)/ -M 755 -O root -G system $$prog; \
	done

clean:
	rm -f $(PROGS) $(TEMPFILES) *.o

include $(ROOT)/Make.libtpty.inc
