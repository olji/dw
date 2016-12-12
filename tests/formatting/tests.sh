#!/bin/sh
. ../util.sh

rm -f lst-format*.log
rm -f lst-format*.log.mleak

assert '$BIN --use-list=incomplete.lst -g5' \
    'grep "Not enough entries" $OUTFILE' \
    0 \
    'LIST FORMAT - Entry completeness' \
    'lst-format_entry-complete.log'
assert '$BIN --use-list=missing_info.lst -g5' \
    'grep "Missing information" $OUTFILE' \
    0 \
    'LIST FORMAT - Missing information' \
    'lst-format_missing-info.log'
assert '$BIN --use-list=charset-len-wrong.lst -g5' \
    'grep "Given character set" $OUTFILE' \
    0 \
    'LIST FORMAT - Unequal character set length' \
    'lst-format_charset-len.log'
assert '$BIN --use-list=missing-line-entry.lst -g5'\
    'grep "Not enough entries present" $OUTFILE' \
    0 \
    'LIST FORMAT - Missing line entry' \
    'lst-format_missing-line-entry.log'
