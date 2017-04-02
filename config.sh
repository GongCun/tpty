#!/bin/sh


DIR=`dirname $0`
TEMPC=${DIR}/temp.c
HEADER=${DIR}/config.h
EXTRALIBS="-lcrypto"
[ "`uname -s`" = "SunOS" ] && EXTRALIBS=$EXTRALIBS" -lsocket"
CC=${CC:-cc}

test -z "`type $CC 2>/dev/null`" && { echo "can't find compiler" >&2; exit 1; }

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
	echo '#define HAVE_OPENSSL 1' >${HEADER}.tmp
	echo 'main(){}' >${TEMPC}
	$CC -o ${TEMPC%.c}.out ${TEMPC} ${LDFLAGS} ${EXTRALIBS} >/dev/null 2>&1 \
	&& echo ${EXTRALIBS}
else
	echo '/* #undef HAVE_OPENSSL */' >${HEADER}.tmp
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
if $CC -o ${TEMPC%.c}.out ${TEMPC} $CPPFLAGS >/dev/null 2>&1; then
echo "#define "`echo HAVE_${func} 1 | tr '[:lower:]' '[:upper:]'`
else
echo "/* #undef "`echo HAVE_${func} | tr '[:lower:]' '[:upper:]'`" */"
fi
} >>${HEADER}.tmp
done

cmp -s $HEADER ${HEADER}.tmp
if [ $? -eq 1 ]
then
	cp ${HEADER}.tmp $HEADER
fi
rm -rf ${TEMPC%.c}.* ${HEADER}.tmp >/dev/null 2>&1

