#!/usr/bin/ksh
TOTAL=$1

i=0
while [ $i -lt $TOTAL ]; do
	echo "i = $i"
	tpty-keygen private.pem
	cp -p test.txt.bk test.txt
	>test.txt.key
	tpty-crypt test.txt assword: ./private.pem.pub ./test.txt.key
	tpty -t -1 -x -f test.txt -p private.pem -k ./test.txt.key ssh ucmtest@gccapco2
	((i+=1))
done
