#!/bin/sh

. ../util.sh

rm invocation_*.log
rm invocation_*.lst

PW_ID=[0-9][0-9][0-9][0-9][0-9]
PW_WORD=[a-zA-Z0-9]*
# Use external list
assert '$BIN -g5 --use-list=list.lst' \
    'grep "$PW_ID\s$PW_ID\s$PW_ID\s$PW_ID\s$PW_ID" $OUTFILE' \
    0 \
    'INVOCATION - Standard generation with --use-list: Passphrase IDs' \
    'invocation_use-list-id.log'
assert '$BIN -g5 --use-list=list.lst' \
    'grep "Passphrase: $PW_WORD\s$PW_WORD\s$PW_WORD\s$PW_WORD\s$PW_WORD" $OUTFILE' \
    0 \
    'INVOCATION - Standard generation with --use-list: Passphrase words' \
    'invocation_use-list-word.log'
# Chaining 1
assert 'echo "y" | $BIN --create-list=src.tmp --use-list=invocation_new_list.lst -g5' \
    'ls | grep new_list.lst' \
    0 \
    'INVOCATION - List create and passphrase generate chaining 1' \
    'invocation_chaining-1.log'
# Chaining 2
assert 'echo "y" | $BIN --use-list=invocation_new_list.lst -g5 --create-list=src.tmp' \
    'ls | grep new_list.lst' \
    0 \
    'INVOCATION - List create and passphrase generate chaining 2' \
    'invocation_chaining-2.log'
# Chaining 3
assert 'echo "y" | $BIN -g5 --create-list=src.tmp --use-list=invocation_new_list.lst' \
    'ls | grep new_list.lst' \
    0 \
    'INVOCATION - List create and passphrase generate chaining 3' \
    'invocation_chaining-3.log'
# Lookup
assert 'echo "42342 12345 83723" | $BIN --use-list=list.lst --lookup' \
    'grep "word24324" output' 0 \
    'INVOCATION - Passphrase lookup' \
    'invocation_lookup.log'
# Character groups
assert 'echo "y" | $BIN --create-list=src.tmp --use-list=invocation_c_groups.lst --use-config=c_groups.conf' \
    'grep "ajibe" invocation_c_groups.lst' \
    0 \
    'INVOCATION - Character set groups' \
    'invocation_cgroup.log'
# Allocation error
assert 'echo "y" | $BIN --create-list=src.tmp --use-list=invocation_alloc_err.lst --use-config=alloc_error.conf' \
    'grep "Allocation error" $OUTFILE' \
    0 \
    'INVOCATION - Memory allocation error' \
    'invocation_allocation-error.log'
