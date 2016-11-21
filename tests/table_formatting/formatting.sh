#!/bin/sh
. ../util.sh

rm -f lst-format_entry_complete.log

assert 'dw -ubad_word_len.l -g5' 1 \
       'LIST FORMAT - Entry completeness' 'lst-format_entry_complete.log'
