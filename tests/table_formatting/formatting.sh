#!/bin/sh
. ../util.sh

rm -f lst-format*.log

assert 'dw --use-list=incomplete.lst -g5' 1 \
    'LIST FORMAT - Entry completeness' 'lst-format_entry-complete.log'
assert 'dw --use-list=missing_info.lst -g5' 1 \
    'LIST FORMAT - Missing information' 'lst-format_missing-info.log
