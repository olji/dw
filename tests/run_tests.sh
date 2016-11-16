#!/bin/sh

CWD=`pwd`

if [[ $CWD != *"tests/" ]] ; then
    cd tests/
fi
for dir in `ls -d */ | sort -n` ; do
    echo ${dir}
    for script in `ls ${dir}*.sh | sort -n` ; do
        echo "${script}"
        cd ${dir}
        ${script}
        echo
    done
    echo
done

cd $CWD
