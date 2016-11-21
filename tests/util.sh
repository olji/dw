#!/bin/sh
# Copyright (C) 2015-2016 R. Jonsson, E. Jansson

DWWD=`pwd | sed 's/\/tests.*$//'`

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'
NOKS=0

succ() {
    echo -e "$*: ${GREEN}PASSED${NC}"
}

fail() {
    echo -e "$*: ${RED}FAILED${NC}"
    NOKS=`expr $NOKS + 1`
}

close() {
    echo "$*: $NOKS NOKS" >> testlog.log 2>&1
}

assert() {
    LOGFILE=$4
    touch $LOGFILE
    COMMANDS=$1
    eval $DWWD/$COMMANDS >> $LOGFILE 2>&1
    STATUSC=$?

    MESSAGE=$3
    EXPECTED=$2
    if [ $STATUSC -eq $EXPECTED ] ; then
        succ $MESSAGE
    else
        echo "#NOK#" >> $LOGFILE 2>&1
        fail $MESSAGE
    fi
}
