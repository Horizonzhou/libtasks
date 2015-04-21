#!/bin/bash
RUN=$1
CFG_DIR=$2
MODE=$3

mkdir -p /tmp/libtasks
cp $CFG_DIR/nikola_tesla.txt /tmp/libtasks

nginx -p $CFG_DIR -c nginx.conf

$RUN $MODE
RC=$?

nginx -p $CFG_DIR -c nginx.conf -s stop

rm /tmp/libtasks/nikola_tesla.txt
rmdir /tmp/libtasks

exit $RC
