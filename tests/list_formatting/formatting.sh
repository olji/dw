#!/bin/sh
. ../util.sh

rm -f lst-format*.log

assert '$BIN --use-list=incomplete.lst -g5' 1 \
    'LIST FORMAT - Entry completeness' 'lst-format_entry-complete.log'
assert '$BIN --use-list=missing_info.lst -g5' 1 \
    'LIST FORMAT - Missing information' 'lst-format_missing-info.log'
