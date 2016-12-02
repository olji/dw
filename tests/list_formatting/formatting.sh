#!/bin/sh
. ../util.sh

rm -f lst-format*.log

assert '$BIN --use-list=incomplete.lst -g5' \
    'grep "Entry not complete" $OUTFILE' \
    0 \
    'LIST FORMAT - Entry completeness' \
    'lst-format_entry-complete.log'
assert '$BIN --use-list=missing_info.lst -g5' \
    'grep "ERR: Missing information" $OUTFILE' \
    0 \
    'LIST FORMAT - Missing information' \
    'lst-format_missing-info.log'
assert '$BIN --use-list=charset-len-wrong.lst -g5' \
    'grep "ERR: Given character set" $OUTFILE' \
    0 \
    'LIST FORMAT - Unequal character set length' \
    'lst-format_charset-len.log'
