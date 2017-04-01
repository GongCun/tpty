ROOT=..
PLATFORM=$(shell $(ROOT)/systype.sh | tr '[:lower:]' '[:upper:]')
EXTRALIBS=$(shell $(ROOT)/config.sh)
ifneq (,$(findstring xlc,$(shell which $(CC) 2>/dev/null)))
include $(ROOT)/Make.defines.xlc
else
include $(ROOT)/Make.defines.gcc
endif
CFLAGS += -D$(PLATFORM)


LIB_NAME = libtpty.a
LIB_OBJS = readn.o writen.o s_pipe.o signal_rest.o signal_intr.o tty_noncanon.o \
	   error.o pty_fork.o pty_open.o strregex.o def_driver.o gettime.o\
	   set_fl.o clr_fl.o lockreg.o locktest.o sys_exit.o fdebug.o

ifneq (,$(findstring -lcrypto,$(EXTRALIBS)))
	LIB_OBJS := $(LIB_OBJS) openssl_rsa.o
endif

all:	$(LIB_NAME)


$(LIB_NAME): $(LIB_OBJS)
	$(AR) rv $(LIB_NAME) $?
	$(RANLIB) $(LIB_NAME)

clean:
	rm -f *.o *~ $(LIB_NAME)

