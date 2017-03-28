i=0
while [ $i -lt 100 ]; do
    echo "i = $i"
    ./tpty  -t -1 -x -f test.txt.mac telnet -l gongcun 192.168.0.110
    ((i+=1))
done
