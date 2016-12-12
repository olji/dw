#!/bin/sh

. ../util.sh

rm config*.log

# Character groups expansion
assert '$BIN -c NONE --use-list=should_not_exist.lst --use-config=c_groups.conf' \
    'grep "Charset: abcdefghij" $OUTFILE' \
    0 \
    'CONFIG - Character group expansion' \
    'config_cgroup.log'
# Character group duplicate removal
assert '$BIN -c NONE --use-list=should_not_exist.lst --use-config=duplicate_cset.conf' \
    'grep "10 duplicate characters removed from" $OUTFILE && grep "Charset: 0123456789" $OUTFILE' \
    0 \
    'CONFIG - Duplicate removal in character set' \
    'config_duplicate.log'
# Character set sorting
assert '$BIN -c NONE --use-list=should_not_exist.lst --use-config=sort_cset.conf' \
    'grep "1234" $OUTFILE' \
    0 \
    'CONFIG - Sorting of character set' \
    'config_cset-sort.log'
