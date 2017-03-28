#!/usr/bin/ksh

i=0
while [ $i -lt 1000 ]; do
	./tpty -t -1 -x -f test.txt ssh ucmtest@gccapco2
	./tpty -t -1 -x -f test.txt2 ssh ucmtest@gccapco2
	./tpty -t -1 -x -f test.txt3 ssh ucmtest@gccapco2
	((i+=1))
done
