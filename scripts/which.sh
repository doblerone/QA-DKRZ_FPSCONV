
# which for path and alias, even if non-interactive

this_which()
{
  local isAdd cm cmd f found i j k last lines
  declare -a cmd f found lines

  # try which
  found=( $( which -a $1 2> /dev/null ) )

  # files which could have aliases
  f[0]=${HOME}/.bashrc

  local saveIFS
  saveIFS="${IFS}"

    for(( i=${#f[*]}-1 ; i > -1 ; --i )) ; do
      # scan for alias. Note such is not effective here
      IFS=$'\r\n'
      lines=( $(grep alias ${f[i]}) )
      IFS="${saveIFS}"

      unset f[i]
      f=( ${f[*]} )
      isAdd=t

      for(( k=0 ; k < ${#lines[*]} ; ++k )) ; do
        cmd=( ${lines[k]} )
        test ${cmd[0]:0:1} = '#' && continue

        # is a file executed by source or . ?
        for(( j=0 ; j < ${#cmd[*]} ; ++j )) ; do
           if [ "${cmd[j]}" = '.' -o "${cmd[j]}" = 'source' ] ; then
              j=$((j+1))
              cm=${cmd[j]//\~/${HOME}}
              if [ -f ${cm} ] ; then
                f[${#f[*]}]=${cm}
                isAdd=f
              fi
           fi
        done

        for(( j=0 ; j < ${#cmd[*]} ; ++j )) ; do
           # search alias
           if [ ${cmd[j]//$1/} != ${1} ] ; then
              # found an alias containing the command in question
              cm=${cmd[j]#*=}
              test ${cm:0:1} = \' -o ${cm:0:1} = '"' && cm=${cm:1}

              last=$((${#cm} - 1 ))
              test ${cm:last} = \' -o ${cm:last} = '"' && cm=${cm:0:last}

              found[${#found[*]}]=${cm}
              break
           fi
        done
      done

#      if [ ${#f[*]} -eq 0 ] ; then
#        echo "no such command or alias $1"
#        return 1
#      fi

      i=${#f[*]}
    done

  echo ${found[*]}
  return 0
}

this_which $1

return $?
