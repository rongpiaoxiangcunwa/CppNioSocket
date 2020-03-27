#!/bin/bash

MYDIR=`dirname $0`
cd $MYDIR
MYDIR=`pwd`

cd $MYDIR/Build
../parallelCompile.py $* --group="$MYDIR/componentGroup.conf" --excludeFile="$MYDIR/excludeCompileComponents"
