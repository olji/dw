#!/bin/sh
# Copyright (C) 2015-2016 R. Jonsson, E. Jansson

CWD=`pwd`

echo
if [[ $CWD != *"tests/" ]] ; then
    cd tests/
    if [[ $? -eq 1 ]] ; then
        echo "Could not move to subdirectory tests, are you not in project directory?"
    fi
fi

SETUP=$1
if [[ $SETUP -eq 1 ]]; then
	sh setup.sh
fi

for dir in `ls -d */ | sort -n` ; do
    cd ${dir}
    for script in `ls *.sh | sort -n` ; do
        echo ${script}
        sh ${script}
    done
    cd -
    echo
done

cd $CWD
