#!/bin/sh

if [[ -z $CWD ]]; then
    CWD=`pwd`
fi
if [[ $CWD != *"tests/" ]] ; then
    cd tests/
    if [[ $? -eq 1 ]] ; then
        echo "Could not move to subdirectory tests, are you not in project directory?"
    fi
fi

. ./util.sh

ID_L=5
SRC_FILE=src_file.tmp
LST_FILE=list.lst
CHAR_S=0123456789
CS_SIZE=`echo $CHAR_S | wc -m`
CS_SIZE=`expr $CS_SIZE - 1`
L_SIZE=1
i=1
while [[ i -le `expr ${ID_L}` ]]; do
    L_SIZE=`expr $L_SIZE \* $CS_SIZE`
    i=`expr $i + 1`
done

if [ -f $SRC_FILE ]; then
    while true; do
        read -p "$SRC_FILE already exists, delete? [y/n]" yn
        case $yn in
            [Yy]* ) rm $SRC_FILE; break;;
            [Nn]* ) SRC_FILE_GEN=no; break;;
        esac
    done
fi

if [[ -z "$SRC_FILE_GEN" ]]; then
    echo "Creating source file suitable for id length $ID_L and character set size $CS_SIZE($L_SIZE words)..."
    i=0
    while [[ i -le `expr ${L_SIZE} - 1` ]]; do
        if [[ $i -ne 0 ]] && [[ `expr $i % 1000` -eq 0 ]]; then
            echo "`expr $i / 1000`000 words complete."
        fi
        echo "word$i" >> $SRC_FILE
        i=`expr $i + 1`
    done
    echo "Source file generation done."
fi

echo "Testing -g option of dw."

# Ensure no prompt will happen during testing, will not be visible
if [ -f $LST_FILE ]; then
    rm $LST_FILE
fi

assert '$BIN --create-list=${SRC_FILE} --use-list=$LST_FILE' 0 'LIST GENERATION - Complete file' 'gen.log'

echo "Creating list files for testing purposes..."
head -n25 $LST_FILE > table_formatting/incomplete.lst
tail -n +3 $LST_FILE > table_formatting/missing_info.lst
cp $LST_FILE invocation/list.lst
cp $SRC_FILE invocation/src.tmp
sed '1s/10/15/' list.lst > list_formatting/charset-len-wrong.lst

cd $CWD
