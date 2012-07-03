#!/bin/bash
echo "Runing $0"
#export PYTHONHOME=/usr:/usr
export PYTHONPATH=/usr/lib64/python2.6/:/usr/lib64/python2.6/lib-dynload:$PYTHONPATH
#export LD_LIBRARY_PATH=/usr/lib64/python2.6/lib-dynload:$LD_LIBRARY_PATH
echo "LIQUIDHOME=$LIQUIDHOME"
echo "PYTHONHOME=$PYTHONHOME"
echo "PYTHONPATH=$PYTHONPATH"
#echo "PATH=$PATH"
#echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

/usr/bin/python $LIQUIDHOME/lib/scripts/xml2afanasy.py $@
