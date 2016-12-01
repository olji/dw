#!/bin/sh

. ../util.sh

rm invocation_*.log

assert '$BIN -g5 --use-list=list.lst' 0 \
    'INVOCATION - Standard generation with --use-list' 'invocation_use-list.log'
assert 'echo "y" | $BIN --create-list=src.tmp --use-list=new_list.lst -g5' 0 \
    'INVOCATION - List create and passphrase generate chaining 1' 'invocation_chaining-1.log'
assert 'echo "y" | $BIN --use-list=new_list.lst -g5 --create-list=src.tmp' 0 \
    'INVOCATION - List create and passphrase generate chaining 2' 'invocation_chaining-2.log'
assert 'echo "y" | $BIN -g5 --create-list=src.tmp --use-list=new_list.lst' 0 \
    'INVOCATION - List create and passphrase generate chaining 3' 'invocation_chaining-3.log'
assert 'echo "42342 12345 83723" | $BIN --use-list=list.lst --lookup | grep "word24324"' 0 \
    'INVOCATION - Passphrase lookup' 'invocation_lookup.log'
assert 'echo "y" | $BIN --create-list=src.tmp --use-list=c_groups.lst --use-config=c_groups.conf && cat c_groups.lst | grep "ajibe"' 0 \
    'INVOCATION - Character set groups' 'invocation_cgroup.log'
assert 'echo "y" | $BIN --create-list=src.tmp --use-list=alloc_err.lst --use-config=alloc_error.conf' 1 \
    'INVOCATION - Memory allocation error' 'invocation_allocation-error.log'
