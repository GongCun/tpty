#!/bin/sh

tpty-keygen key
tpty-crypt test.cfg assword key.pem.pub keystore
tpty -n -xc ./test.cfg -p ./key.pem -k ./keystore ssh $1

