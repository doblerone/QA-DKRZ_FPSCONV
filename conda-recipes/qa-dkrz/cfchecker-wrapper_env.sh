#!/bin/bash

get_QA_path()
{
   local i items p src
   declare -a items

   p=$0

   while [ -h $p ] ; do
      # resolve symbolic links: cumbersome but robust,
      # because I am not sure that ls -l $p | awk '{print $11}'
      # works for any OS

      items=( $(ls -l $p) )
      i=$((${#items[*]}-1))
      p=${items[i]}
   done

   p=${p%/*}

   # resolve relative path
   if [ ${p:0:1} != '/' ] ; then
     cd $p &> /dev/null
     p=$(pwd)
     cd - &> /dev/null
   fi

   QA_PATH=${p%/*}

   return
}

unset LD_LIBRARY_PATH

get_QA_path

exec ${QA_PATH}/opt/qa-dkrz/scripts/dkrz-cf-checker $@
