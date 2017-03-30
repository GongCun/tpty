#!/bin/sh

DIR=`dirname $0`
TEMPC=${DIR}/temp.c
HEADER=${DIR}/config.h
EXTRALIBS="-lcrypto"
CC=${CC:-cc}

which $CC >/dev/null 2>&1 || { echo "can't find compiler" >&2; exit 1; }

cat >${TEMPC} <<!
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
!

if $CC -E ${TEMPC} ${CPPFLAGS} >/dev/null 2>&1
then
	echo '#define HAVE_OPENSSL 1' >$HEADER
	echo 'main(){}' >${TEMPC}
	cc -o ${TEMPC%.c}.out ${TEMPC} ${LDFLAGS} ${EXTRALIBS} >/dev/null 2>&1 \
	&& echo ${EXTRALIBS}
else
	echo '/* #undef HAVE_OPENSSL */' >$HEADER
fi

for func in unlockpt posix_openpt ptsname grantpt nanosleep
do
cat >${TEMPC} <<!
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
typedef char func(void);
func *f = (func *)$func;
int main(void) { return(f != (func *)$func); }
!
{
if $CC -o ${TEMPC%.c}.out ${TEMPC} >/dev/null 2>&1; then
echo "#define "`echo HAVE_${func} 1 | tr 'a-z' 'A-Z'`
else
echo "/* #undef "`echo HAVE_${func} | tr 'a-z' 'A-Z'`" */"
fi
} >>$HEADER
done

rm -rf ${TEMPC%.c}.* >/dev/null 2>&1

