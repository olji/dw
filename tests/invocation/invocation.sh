#!/bin/sh

. ../util.sh

rm invocation_*.log

assert 'dw -g5 --use-list=list.lst' 0 \
    'INVOCATION - Standard generation with --use-list' 'invocation_use-list.log'
assert 'dw --create-list=src.tmp --use-list=new_list.lst -g5' 0 \
    'INVOCATION - List create and passphrase generate chaining 1' 'invocation_chaining-1.log'
assert 'dw --use-list=new_list.lst -g5 --create-list=src.tmp' 0 \
    'INVOCATION - List create and passphrase generate chaining 2' 'invocation_chaining-2.log'
assert 'dw -g5 --create-list=src.tmp --use-list=new_list.lst' 0 \
    'INVOCATION - List create and passphrase generate chaining 3' 'invocation_chaining-3.log'
assert 'echo "42342 12345 83723" | dw --use-list=list.lst --lookup | grep "word24324"' 0 \
    'INVOCATION - Passphrase lookup' 'invocation_lookup.log'

