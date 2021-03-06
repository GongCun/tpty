ROOT=..
PLATFORM=$(shell $(ROOT)/systype.sh)
EXTRALIBS=$(shell $(ROOT)/config.sh)
include $(ROOT)/Make.defines.gcc
CFLAGS += -D$(PLATFORM)

PROGS =	sig_test time_test fscanf_test strlen_test atoi_test struct_test \
	char_test getc_test wait_test child_test str_test fdebug_test \
	regex_test fread_test sig_test02 err_test compile_test gettime_test \
	lock_test delim_test

ifneq (,$(findstring -lcrypto,$(EXTRALIBS)))
	PROGS := $(PROGS) encrypt_test decrypt_test
endif


all:	$(PROGS)

sig_test: sig_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o sig_test sig_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

time_test: time_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o time_test time_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

fscanf_test: fscanf_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o fscanf_test fscanf_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

strlen_test: strlen_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o strlen_test strlen_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

atoi_test: atoi_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o atoi_test atoi_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

struct_test: struct_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o struct_test struct_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

char_test: char_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o char_test char_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

getc_test: getc_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o getc_test getc_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

wait_test: wait_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o wait_test wait_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

child_test: child_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o child_test child_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

str_test: str_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o str_test str_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

fdebug_test: fdebug_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o fdebug_test fdebug_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

regex_test: regex_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o regex_test regex_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

encrypt_test: encrypt_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ encrypt_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

decrypt_test: decrypt_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ decrypt_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

fread_test: fread_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ fread_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

sig_test02: sig_test02.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o sig_test02 sig_test02.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

compile_test: compile_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ compile_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

err_test: err_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ err_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

gettime_test: gettime_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ gettime_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

lock_test: lock_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ lock_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

delim_test: delim_test.o $(LIBTPTY)
	$(CC) $(CFLAGS) -o $@ delim_test.o $(LDFLAGS) $(LDLIBS) $(EXTRA)

clean:
	rm -f $(PROGS) $(TEMPFILES) *.o a.out tpty*

include $(ROOT)/Make.libtpty.inc
