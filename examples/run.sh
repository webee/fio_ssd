#!/bin/sh
cmd=$1
n=$2

for ((i=0;i<$n;i++));do
    eval $cmd
    if [ $? != 0 ];then
        echo "!ERROR"
        exit $i
    fi
done
echo "DONE!"

