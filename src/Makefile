ROOT=..
PLATFORM=$(shell $(ROOT)/systype.sh | tr 'a-z' 'A-Z')
EXTRALIBS=$(shell $(ROOT)/config.sh)
ifneq (,$(findstring xlc,$(shell which $(CC) 2>/dev/null)))
include $(ROOT)/Make.defines.xlc
else
include $(ROOT)/Make.defines.gcc
endif
CFLAGS += -D$(PLATFORM)

ifneq (,$(findstring -lcrypto,$(EXTRALIBS)))
	PROGS =	tpty tptyreplay tpty-crypt
else
	PROGS =	tpty tptyreplay
endif

all:	$(PROGS)

tpty:	main.o loop.o do_driver.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LDFLAGS) $(LDLIBS)
	@cp -p tpty ../bin
	@cp -p tpty ../dbg

tptyreplay: tptyreplay.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LDFLAGS) $(LDLIBS)
	@cp -p tptyreplay ../bin
	@cp -p tptyreplay ../dbg

tpty-crypt: tpty-crypt.o
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LDFLAGS) $(LDLIBS)
	@cp -p tpty-crypt ../bin
	@cp -p tpty-crypt ../dbg

clean:
	rm -f $(PROGS) $(TEMPFILES) *.o

include $(ROOT)/Make.libtpty.inc
