
# which for path and alias, even if non-interactive

which()
{
  local isAdd cm cmd f i j last
  declare -a cmd f

  f[0]=${HOME}/.bashrc

  if ! cmd=( $(/usr/bin/which $1 2> /dev/null) ) ; then
    for(( i=${#f[*]}-1 ; i > -1 ; --i )) ; do
      # scan for alias. Note such is not effective here
      cmd=( $(grep alias ${f[i]}) )
      unset f[${i}]
      f=( ${f[*]} )
      isAdd=t

      for(( j=1 ; j < ${#cmd[*]} ; ++j )) ; do
        cm=${cmd[j]}
        test ${cm:0:1} = '~' && cm=${HOME}${cm:1}

        if [ ${isAdd} = t -a -f ${cm} ] ; then
          f[${#f[*]}]=${cm}
          isAdd=f
        else
          test ${cmd[j]} = 'alias'  && continue
          test ${cmd[$((j-1))]} != 'alias'  && continue

          cm=${cmd[j]#*=}
          test ${cm:0:1} = \' -o ${cm:0:1} = '"' && cm=${cm:1}

          last=$((${#cm} - 1 ))
          test ${cm:last} = \' -o ${cm:last} = '"' && cm=${cm:0:last}

          if [ ${cm##*/} = $1 ] ; then
            cmd=${cm}
            break 2
          fi
        fi
      done

      if [ ${#f[*]} -eq 0 ] ; then
        echo "no such command or alias $1"
        exit 1
      fi

      i=${#f[*]}
    done
  fi

  echo "${cmd}"
  return
}

which $1
