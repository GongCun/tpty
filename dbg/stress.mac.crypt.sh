TOTAL=$1
i=0
while [ $i -lt $TOTAL ]; do
    echo "i = $i"
    ./tpty-keygen ./private.pem
    # openssl genrsa -out ./private.pem 2048
    # openssl rsa -in ./private.pem -outform PEM -pubout -out ./public.pem
    cp -p test.txt.mac.bk test.txt.mac
    >./test.txt.mac.key
    ./tpty-crypt test.txt.mac assword: ./private.pem.pub ./test.txt.mac.key
    ./tpty-crypt test.txt.mac Password: ./private.pem.pub ./test.txt.mac.key
    ./tpty -t -1 -x -f test.txt.mac -p ./private.pem -k test.txt.mac.key \
        telnet -l gongcun 192.168.0.110
    ((i+=1))
done
