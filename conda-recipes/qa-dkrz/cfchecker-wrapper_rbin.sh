#!/bin/bash

# note that this scripts will be written into conda-root/bin

# path to the sources
export QA_SRC

# resolve (multiple linked) symbolic link
QA_SRC=${0%/*}
QA_SRC=${QA_SRC%/*}

declare -a fs items

# try a search in QA_SRC, which could be conda or not.
fs=( $( find ${QA_SRC}/envs -type d -name qa-dkrz ) )

for f in ${fs[*]} ; do
  items=( ${f//\// } )
  for(( i=0 ; i < ${#items[*]} ; ++i )) ; do
    QA_SRC=
    if [ ${items[i]} = envs ] ; then
      for(( j=0 ; j < i+2 ; ++j )) ; do
        QA_SRC=${QA_SRC}/${items[j]}
      done
      QA_SRC=${QA_SRC}/opt/qa-dkrz
      break 2
    fi
  done
done

if [ -f ${QA_SRC}/scripts/dkrz-cf-checker ] ; then
  exec ${QA_SRC}/scripts/dkrz-cf-checker $@
else
  echo "no such file ${QA_SRC}/scripts/dkrz-cf-checker"
fi
