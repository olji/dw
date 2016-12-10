#!/bin/sh
# Copyright (C) 2015-2016 R. Jonsson, E. Jansson

BIN=`pwd | sed 's/\/tests.*$//'`/dw

OUTFILE=output
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
    LOGFILE=$5
    touch $LOGFILE
    RUN_COMMANDS=$1
    CHECK_COMMANDS=$2
    eval "$RUN_COMMANDS > $OUTFILE 2>&1"
    cat $OUTFILE > $LOGFILE
    if [[ ! -z "${CHECK_COMMANDS}" ]]; then
        echo >> $LOGFILE
        echo >> $LOGFILE
        echo "#CHECKING#" >> $LOGFILE
        eval "$CHECK_COMMANDS >> $LOGFILE 2>&1"
    fi
    STATUSC=$?

    MESSAGE=$4
    EXPECTED=$3
    if [ $STATUSC -eq $EXPECTED ] ; then
        succ $MESSAGE
    else
        echo "#NOK#" >> $LOGFILE 2>&1
        fail $MESSAGE
    fi

    echo "" > $OUTFILE
    eval "valgrind --log-file=$OUTFILE $RUN_COMMANDS > /dev/null 2>&1"
    cat $OUTFILE > $LOGFILE.mleak
    grep "in use at exit: 0 bytes" $OUTFILE >> $LOGFILE.mleak 2>&1
    if [ $? -eq 0 ] ; then
        succ "Memory leaks"
    else
        fail "Memory leaks"
    fi
    grep "ERROR SUMMARY: 0" $OUTFILE >> $LOGFILE.mleak 2>&1
    if [ $? -eq 0 ] ; then
        succ "Valgrind errors"
    else
        fail "Valgrind errors"
    fi
    echo
    rm $OUTFILE
}
