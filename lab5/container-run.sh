#!/bin/bash

if (( $#<2 ))
then
  echo "wrong number of arguments"
  exit 1
fi

if [ ! -f $1 ]
then
  echo "config file doesn't exist"
  exit 1
fi

CONTAINER_ROOT_DIR="/tmp/bash-container"
mkdir -p $CONTAINER_ROOT_DIR

CMD=""
i=0
for arg in "${@:2}"
do
  CMD=$CMD" "$arg
done
echo $CMD

i=1 
while [[ $i<$( wc -l $1 ) ]]
do
  line=$( head -n $i $1 | tail -n 1 )
  OG_PATH=$( echo $line | cut -d " " -s -f 1 )
  CONT_PATH=$CONTAINER_ROOT_DIR$( echo $line | cut -d " " -s -f 2 )
  if [[ -d $OG_PATH ]]
  then
    mkdir -p $CONT_PATH
    bindfs --no-allow-other $OG_PATH $CONT_PATH
  else
    mkdir -p $( dirname $CONT_PATH )
    cp $OG_PATH $CONT_PATH
  fi
  (( i++ ))
done

fakechroot chroot $CONTAINER_ROOT_DIR $CMD

exit 0
