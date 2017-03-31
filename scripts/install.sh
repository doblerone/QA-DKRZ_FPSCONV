#! /bin/bash

# ============== Please, do not edit this file ===================
package=QA-DKRZ  # by default
prg=install.sh

MAKEFILE=Makefile

compilerSetting()
{
  # Notice priority
  local locCC locCFLAGS locCXX locCXXFLAGS
  local lf

  # external setting gets highest priority
  locCC="$CC"
  locCFLAGS="$CFLAGS"
  locCXX="$CXX"
  locCXXFLAGS="$CXXFLAGS"

  # Anything given in the install_configure file?
  if [ -f install_configure ] ; then
    . install_configure
  fi

  if [ ${#locCC} -gt 0 ] ; then
    CC="${locCC}"
  fi
  if [ ${#locCFLAGS} -gt 0 ] ; then
    CFLAGS="${locCFLAGS}"
  fi
  if [ ${#locCXX} -gt 0 ] ; then
    CXX="${locCXX}"
  fi
  if [ ${#locCXXFLAGS} -gt 0 ] ; then
    CXXFLAGS="${locCXXFLAGS}"
  fi

  # no external setting, try for gcc/g++
  local tmp
  if [ ${#CC} -eq 0 ] ; then
     if tmp=$(which gcc) ; then
        CC=$tmp
        log "default: export CC=${tmp}" DONE
     fi
  fi

  if [ ${#CFLAGS} -eq 0 ] ; then
    CFLAGS="-O2"
    log "default: export CFLAGS=-O2" DONE
  fi

  if [ ${#CXX} -eq 0 ] ; then
     if tmp=$(which g++) ; then
        CXX=$tmp
        log "default: export CXX=${tmp}" DONE
     fi
  fi

  if [ ${#CXXFLAGS} -eq 0 ] ; then
    CXXFLAGS="-O2"
    log "default: export CXXFLAGS=-O2" DONE
  fi

  if [ ! -f install_configure ] ; then

    cp .install_configure install_configure
    log "create install_configure" DONE
  fi

  if diff -q install_configure .install_configure &> /dev/null ; then
    if [ ${isBuild:-f} = f -a ${isLink:-f} = f ] ; then
      echo "Please, edit file install_configure."
      exit 41
    fi
  fi

  FC=""
  F90=""

  return
}

descript()
{
  echo "usage: scripts/install.sh"
  echo "  -B                Unconditionally re-make all"
  echo "  -d                Execute 'make' with debugging info."
  echo "  --build           Download and build required libraries."
  echo "  --debug[=script]  Display execution commands."
  echo "  --help            Also option -h."
  echo "  --link=path       Link lib and include files, respectively, of external netcdf"
  echo "                    to the directories 'your-path/${package}/local'."
  echo "  --package=str     str=QA-version."
  echo "  --show-inst       Display properties of the current installation."
  echo "  --src-path=path   To the place were all three libs reside."
  echo "    project-name    At present CMIP5 and CORDEX."
  echo "                    Installation for both by default. Note: no '--'."
}

formatText()
{
  # format text ready for printing

  # date and host
  local k N n str0 str isWrap

  str0="$*"

  # The total output is subdivided into chunks of pmax characters.
  # Effect of \n is preserved.
  N=$pEnd  # special: taken from log()
  str=

  while : ; do
    k=0  # necessary when skipping the loop

    if [ ${isWrap:-f} = t ] ; then
      n=$(( N - 6 ))
    else
      n=$N
    fi

    if [ ${#str0} -ge $n ] ; then
      # wrap lines with length > N
      for (( ; k < n ; ++k )) ; do
        if [ "${str0:k:2}" = "\n" ] ; then
          str="${str}${str0:0:k}\n"
          str0=${str0:$((k+2))}
          isWrap=f
          continue 2
        fi
      done
    fi

    # the last line
    if [ ${#str0} -le $n ] ; then
      str="${str}${str0}"
      break

    # sub-lines length equals N
    elif [ $k -eq $n -a "${str0:k:2}" = "\n" ] ; then
      str="${str}${str0:0:n}"
      str0=${str0:n}

    # wrap line
    else
      str="${str}${str0:0:n}\n      "
      str0=${str0:n}
      isWrap=t
    fi
  done

  if [ ${isWrap:-f} = t ] ; then
    lastLineSz=$(( ${#str0} + 6 ))
  else
    lastLineSz=${#str0}
  fi

  formattedText=${str}
}

getRevNum()
{
  # get current revision number; this determines whether it is
  # before a change of defaults happened.
  local arg=$1

  local branch=$(git branch | grep '*' | awk '{print $2}')
  local currIdent=$(git log --pretty=format:'%h' -n 1)

  eval ${arg}=${branch}-${currIdent}

  return
}

libInclSetting()
{
   export CC CXX CFLAGS CXXFLAGS FC F90
   export LIB INCLUDE

   local i

   if [ ${isBuild:-f} = t -o ${isLink:-f} = t ] ; then
     # install zlib, hdf5, and/or netcdf.
     # LIB and INCLUDE, respectively, are colon-separated singles
     if [ ${isBuild:-f} = t -o ${isLink:-f} = t ] && \
             bash scripts/install_local ${coll[*]} --saveLocal \
                  ${isBuild:+--build} ${isLink:+--link} ; then

      isBuild=
      isLink=
     else
      echo 'no path to at least one of netCDF, hdf, zlib, udunits2,'
      echo 'please, inspect files QA_SRC/local/source/INSTALL_*.log'
      exit 41
     fi

     compilerSetting

     store_LD_LIB_PATH
   fi

   LIB="${LIB/#/-L}"
   LIB="${LIB/ / -L}"
   LIB="${LIB//:/ -L}"

   INCLUDE=${INCLUDE/#/-I}
   INCLUDE=${INCLUDE/ / -I}
   INCLUDE=${INCLUDE//:/ -I}

   local is

   local is_i=( f f f f )
   local is_l=( f f f f )
   local item

   # ----check include file
   for item in ${INCLUDE[*]} ; do
     test -e ${item:2}/zlib.h && is_i[0]=t
     test -e ${item:2}/hdf5.h && is_i[1]=t
     test -e ${item:2}/netcdf.h && is_i[2]=t
     test -e ${item:2}/udunits2.h && is_i[3]=t
   done

   # ----check lib
   # note that static and/or shared as well as lib and/or lib64 may occur
   local j tp typ kind isShared isStatic
   typ=( a so )

   local isShared=( f f f f )
   local isStatic=( f f f f )

   for tp in a so ; do
     for item in ${LIB[*]} ; do
       if [ -e ${item:2}/libz.${tp} ] ; then
          is_l[0]=t

          if [ ${tp} = a ] ; then
            isStatic[0]=t
          else
            isShared[0]=t
          fi
       fi
       if [ -e ${item:2}/libhdf5.${tp} ] ; then
          is_l[1]=t

          if [ ${tp} = a ] ; then
            isStatic[1]=t
          else
            isShared[1]=t
          fi
       fi
       if [ -e ${item:2}/libnetcdf.${tp} ] ; then
          is_l[2]=t

          if [ ${tp} = a ] ; then
            isStatic[2]=t
          else
            isShared[2]=t
          fi
       fi
       if [ -e ${item:2}/libudunits2.${tp} ] ; then
          is_l[3]=t

          if [ ${tp} = a ] ; then
            isStatic[3]=t
          else
            isShared[3]=t
          fi
       fi
     done
   done

   local isS=f
   for(( i=0 ; i < ${#isShared[*]} ; ++i )) ; do
      test ${isShared[i]} = ${isStatic[i]} && isStatic[i]=f
      test ${isStatic[i]} = t && isS=t
   done

   if [ ${isS:-f} = t ] ; then
      local dls
      dls=( $( find /lib -name "libdl.*" 2> /dev/null ) )
      test ${#dls[*]} -eq 0 && \
          dls=( $( find /usr/lib -name "libdl.*" 2> /dev/null ) )

      test ${#dls[*]} -gt 0 && export LIBDL='-ldl'
   fi

  local isI=t
  local isL=t

  for(( i=0 ; i < ${#is_i[*]} ; ++i )) ; do
    test ${is_i[i]} = f && isI=f
    test ${is_l[i]} = f && isL=f
  done

  if [ ${isI} = f -o ${isL} = f ] ; then
     echo "At least one path in"
     for(( i=0 ; i < ${#LIB[*]} ; ++i )) ; do
       echo -e "\t ${LIB[i]}"
     done
     echo -e "or \t ${INCLUDE[*]}"
     echo "is broken."

     showInst
     exit 1
  fi


  return
}

log()
{
  test ${isDebug:-f} = t && set +x

  # get status from last process
  local status=$?

  local n p
  local pEnd=80
  local str DONE
  local term

  if [ ${isContLog:-f} = f ] ; then
    echo -e "\n$(date +'%F_%T'):" >> ${logPwd}install.log
    isContLog=t
  fi

  n=$#
  local lastWord=${!n}
  if [ "${lastWord:0:4}" = DONE ] ; then
    term=${lastWord#*=}
    n=$(( n - 1 ))
    status=0
  elif [ "${lastWord}" = FAIL ] ; then
    term=FAIL
    n=$(( n - 1 ))
    status=1
  elif [ ${status} -eq 0 ] ; then
    term=DONE
  else
    term=FAIL
  fi

  str=
  for(( p=1 ; p <= n ; ++p )) ; do
    str="${str} ${!p}"
  done

  formatText "${str}"
  echo -e -n "$formattedText" >> ${logPwd}install.log

  if [ ${#term} -gt 0 ] ; then
    str=' '
    for(( p=${lastLineSz} ; p < ${pEnd} ; ++p )) ; do
      str="${str}."
    done
    str="${str} ${term}"
  fi

  echo "$str" >> ${logPwd}install.log

  test ${isDebug:-f} = t && set -x

  return $status
}

makeProject()
{
  PROJECT=$1
  local cxxFlags="${CXXFLAGS}"
  local CXXFLAGS
  test "${PROJECT_AS}" && PROJECT=${PROJECT_AS}

  export PROJECT=$PROJECT

  if [ ${PROJECT} = CF ] ; then
    local cfc=CF-checker
    CXXFLAGS="${cxxFlags} -D CF_MACRO"

  elif [ ${PROJECT} = MODIFY ] ; then
    MAKEFILE=Makefile_modify
    local cfc=ModifyNc

  else
    export QA_PRJ_HEADER=qa_${PROJECT}.h
    export QA_PRJ_SRC=QA_${PROJECT}.cpp
    CXXFLAGS="${cxxFlags} -D ${PROJECT} "

    unset cfc

    if [ $(ps -ef | grep -c qa-DKRZ) -gt 1 ] ; then
      # protect running sessions, but not really thread save
      export PRJ_NAME=qqA-${PROJECT}
      test -f $BIN/qA-${PROJECT}.x && \
        cp -a $BIN/qA-${PROJECT}.x $BIN/${PRJ_NAME}.x
    else
      # nothing to protect
      export PRJ_NAME=qA-${PROJECT}
    fi
  fi

  if ! make ${always} -q -C $BIN -f ${QA_SRC}/$MAKEFILE ${cfc} ; then
     # not upto-date
     if make ${always} ${mk_D} -C $BIN -f ${QA_SRC}/$MAKEFILE ${cfc} ; then
       test ${PROJECT} != CF && log "make qa-${PROJECT}.x" DONE
     else
       test ${PROJECT} != CF && log "make qa-${PROJECT}.x" FAIL
       exit 1
     fi
  fi

  if [ ${PROJECT} != CF -a ${PROJECT} != MODIFY ] ; then
    test ${PRJ_NAME:0:3} = qqA && mv $BIN/${PRJ_NAME}.x $BIN/qA-${PROJECT}.x
  fi

  return
}

makeUtilities()
{
  # small utilities
  if make ${always} -q -C $BIN -f ${QA_SRC}/$MAKEFILE c-prog cpp-prog ; then
    status=$?
#    log "C/C++ utilities" DONE=up-to-date
  else
    # not up-to-date
    # executes with an error, then again
    text=$(make ${always} ${mk_D} \
            -C $BIN -f ${QA_SRC}/$MAKEFILE c-prog cpp-prog 2>&1 )
    if [ $? -gt 0 ]; then
      export CFLAGS="${CFLAGS[*]} -DSTATVFS"
      text=$(make ${always} ${mk_D} \
            -C $BIN -f ${QA_SRC}/$MAKEFILE c-prog cpp-prog 2>&1 )
    fi

    if [ $? -eq 0 ] ; then
      test ${displayComp:-f} = t && echo -e "${text}"
      log "C/C++ utilities" DONE
    else
      echo -e "$text"
      log "C/C++ utilities" FAIL
      exit 1
    fi
  fi

  return
}

saveAsCycle()
{
  local keep=f

  for f in $* ; do
    if [ ${f} = KEEP-A-COPY ] ; then
       keep=t
       continue
    fi

    if [ ! \( -f $f -o -d $f \) ] ; then
      echo "install.saveAsCycle: no such file or directory $f"
      return
    fi

    local ext val x
    local maxVal fs fx

    maxVal=0
    fs=( $(ls -d $f.* 2> /dev/null) )

    for fx in ${fs[*]} ; do
      ext=${fx##*.}

      if val=$(expr match $ext '\([[:digit:]]\+$\)' 2> /dev/null) ; then
        test ${val:-0} -gt ${maxVal} && maxVal=$val
      fi
    done

    if [ ${keep} = t ] ; then
      cp $f ${f##*/}.$((++maxVal)) 2> /dev/null
    else
      mv $f ${f##*/}.$((++maxVal)) 2> /dev/null
    fi
  done
}

showInst()
{
   echo "State of current QA Installation:"
   echo "======================="
   echo "CC=$CC"
   echo "CXX=$CXX"
   echo "CFLAGS=$CFLAGS"
   echo "CXXFLAGS=$CXXFLAGS"
   echo "OSTYPE=$OSTYPE"
   echo "BASH=$(bash --version | head -n 1)"
   echo -e "\nBIN=${BIN}: "
   fs=($( ls -d ${BIN}/*))
   if [ ${#fs[*]} -gt 1 ] ; then
     echo -e "\t${fs[*]##*/}"
   fi

   fs=($( ls -d ${QA_SRC}/local/*))
   echo -e "\nQA/local:"
   for f in ${fs[*]##*/} ; do
     echo -e "\t$f"
   done

   if fs=($( ls -d ${QA_SRC}/local/source/* 2> /dev/null)) ; then
     echo "QA/local/source:"

     for f in ${fs[*]##*/} ; do
       echo -e "\t$f"
     done
   fi

   local is=( f f f f )
   local text=( "zlib" "hdf5" "netCDF" "udunits" )

   local item

   # ----check include file
   echo -e "\nINCLUDE=${INCLUDE}"

   for item in ${INCLUDE} ; do
     test -e ${item:2}/zlib.h && is[0]=t
     test -e ${item:2}/hdf5.h && is[1]=t
     test -e ${item:2}/netcdf.h && is[2]=t
     test -e ${item:2}/udunits2.h && is[3]=t
   done

   local i
   for(( i=0 ; i < ${#is[*]} ; ++i )) ; do
     echo -e -n "\t${text[i]}\t: "
     if [ ${is[i]} = t  ] ; then
        echo "yes"
      else
        echo "no"
      fi
   done

   # ----check lib files

   echo -e "\nLIB=${LIB}"

   local j typ kind
   typ=( a so )
   kind=( static shared )

   for(( j=0 ; j < ${#typ[*]} ; ++j )) ; do
     is=( f f f f )

     for item in ${LIB[*]//:/ } ; do
       test -e ${item:2}/libz.${typ[j]} && is[0]=t
       test -e ${item:2}/libhdf5_cpp.${typ[j]} && is[1]=t
       test -e ${item:2}/libnetcdf.${typ[j]} && is[2]=t
       test -e ${item:2}/libudunits2.${typ[j]} && is[3]=t
     done

     for(( i=0 ; i < ${#is[*]} ; ++i )) ; do
       echo -e -n "\t${text[i]} (${kind[j]})\t: "
       if [ ${is[i]} = t ] ; then
         echo "yes"
       else
         echo "no"
       fi
     done
   done

   # ----check ncdump executable
   is=f
   for item in ${LIB[*]//:/ } ; do
     item=${item:2}
     if [ -e ${item%/*}/bin/ncdump ] ; then
       is=t
       break
     fi
   done

   if [ ${is} = t ] ; then
     echo -e "\nNCDUMP=$( ${item%/*}/bin/ncdump 2>&1 | tail -n 1 )"
   else
     echo -e "\nncdump: no such file"
   fi

   # -------------------

   cd $QA_SRC &> /dev/null

#   local LANG=en_US
#   echo ''
#   svn info

   test ${isShowInstFull:-f} = f && return

   echo -e "\ninstall_configure:"
   cat install_configure

   echo -e "\nQA Directories/Files"
   echo -e "\n${package}:"
   ls -d *
   echo -e "\n${package}/bin:"
   ls -l bin/*
   echo -e "\n${package}/include:"
   ls -l include/*
   echo -e "\n${package}/src:"
   ls -l src/*

   if [ -e "local" ] ; then
     echo -e "\n${package}/local:"
     ls -d local/*
   fi

   if [ -e local/lib ] ; then
     echo -e "\n${package}/local/lib:"
     ls -ld local/lib/*
   fi

   return
}

store_LD_LIB_PATH()
{
  # paths to shared libraries

  # current -L paths
  local i j lib
  lib=( ${LIB[*]//-L/} )
  lib=( ${lib[*]//:/ } )

  local ldp
  for(( i=0 ; i < ${#lib[*]} ; ++i )) ; do
    test ${#ldp} -gt 0 && ldp=${ldp}:
    ldp=${ldp}${lib[i]}
  done

  # store/update LD_LIBRARY_PATH in .conf
  . $QA_SRC/scripts/updateConfigFile.txt LD_LIBRARY_PATH=${ldp}

  return
}

tr_option()
{
  local phrase="${!1}"

  local sz i
  local sz=${#phrase}
  for((i=0 ; i < sz ; ++i )) ; do
    test "${phrase:i:1}" = '=' && break
  done

  if [ ${i} -eq ${sz} ] ; then
    phrase=$( echo "${phrase/% /}" | tr "[:lower:]" "[:upper:]" )
  else
    local tmp0="${phrase:0:i}"
    tmp0="${tmp0//QC/QA}"
    tmp0=$( echo "${tmp0/% /}" | tr "[:lower:]" "[:upper:]" )
    phrase="${tmp0}""${phrase:i}"
  fi

  eval ${1}=\${phrase}
  return
}


######### main ##########

while getopts Bdhq:-: option ${args[*]}
do
  UOPTARG="$OPTARG"
  tr_option UOPTARG
  OPTNAME=${UOPTARG%%=*}
  OPTVAL=${OPTARG#*=}

  case $option in
    B)  always=-B ;;                # unconditionally make all
    d)  mk_D=-d ;;                   # make with debugging info
    h)  descript
        exit 41 ;;
    q)  QA_SRC=${OPTARG} ;;
    -)  if [ "${OPTNAME}" = CONTINUE_LOG ] ; then
           isContLog=t
        elif [ "${UOPTARG:0:5}" = BUILD ] ; then
           # make libraries in ${package}/local
           isBuild=t
           continue
        elif [ ${UOPTARG:0:5} = DEBUG -o ${UOPTARG} = 'DEBUG=install.sh' ] ; then
           set -x
           isDebug=t
        elif [ "${OPTNAME}" = DISPLAY_COMP ] ; then
           displayComp=t
           continue
        elif [ "${UOPTARG:0:4}" = LINK ] ; then
           isLink=t
        elif [ "${UOPTARG:0:7}" = NO_COMP ] ; then
          isCompilation=f
        elif [ "${OPTNAME}" = PACKAGE ] ; then
           package=${OPTVAL}
        elif [ "${OPTNAME}" = PROJECT_AS ] ; then
          PROJECT_AS="${OPTVAL}"
        elif [ "${OPTNAME}" = QA_SRC ] ; then
          QA_SRC=${OPTVAL}
        elif [ "${OPTNAME}" = DEFAULT_PROJECT ] ; then
           defaultProject=${OPTVAL}
        elif [ "${OPTNAME}" = SHOW-INST ] ; then
           isShowInst=t
           test "${OPTVAL}" = FULL && isShowInstFull
           continue
        fi

        coll[${#coll[*]}]=--${OPTARG}
        ;;
   \?)  descript
        echo -e "\nscripts/install.sh: undefined option ${option}"
        exit 41;;
  esac
done

shift $(( $OPTIND - 1 ))

cd ${QA_SRC}

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${QA_SRC}/local/lib64:${QA_SRC}/local/lib
#export LD_LIBRARY_PATH=/home/hdh/miniconda/lib
export LD_RUN_PATH=${LD_LIBRARY_PATH}

# compiler settings
# import setting; these have been created during first installation
compilerSetting

# existence, linking or building
libInclSetting

BIN=${QA_SRC}/bin

test ! -d $BIN && mkdir bin

# Note that any failure in a function called below causes an EXIT

if [ ${isShowInst:-f} = t ] ; then
  showInst
  exit 41
fi

test ${isCompilation:-t} = f && exit 0

# c/c++ stand-alone programs
test ${#QA_LIBS} -eq 0 && \
  export QA_LIBS="-ludunits2 -lnetcdf -lhdf5_hl -lhdf5 -lz -luuid "

test ${#QA_LIBS_ADD} -gt 0 && \
  export QA_LIBS="${QA_LIBS} ${QA_LIBS_ADD}"


makeUtilities

# check projects' qa executables
if [ $# -eq 0 ] ; then
  projects=( ${defaultProject:-CORDEX} )
else
  projects=( $* )
fi

# the revision number is inserted in Makefile via -DREVISION
getRevNum REVISION
export REVISION

# list of all QA C++ source files and project key-words; the latter
# are given as pseudo file name
prj_cpp=( $(ls $QA_SRC/src/QA_*.cpp ) QA_CF.cpp QA_MODIFY.cpp )
prj_cpp=( ${prj_cpp[*]##*/} )

# rm those that are not project realted
non_prj_cpp=( QA_cnsty.cpp QA_data.cpp QA_DRS_CV_Table.cpp QA_main.cpp QA_time.cpp )

for(( i=0 ; i < ${#prj_cpp[*]} ; ++i )) ; do
  for(( j=0 ; j < ${#non_prj_cpp[*]} ; ++j )) ; do
    if [ "${prj_cpp[i]}" ==  ${non_prj_cpp[j]} ] ; then
      unset prj_cpp[i]
      break
    fi
  done
done

prj_cpp=( ${prj_cpp[*]} )

if [ ${#PROJECT_AS[*]} -gt 0 -a "${PROJECT_AS}" != "${projects[*]}" ] ; then
  prjs=(${PROJECT_AS})
else
  prjs=( ${projects[*]} )
fi

for prj in ${prjs[*]} ; do
  for(( i=0 ; i < ${#prj_cpp[*]} ; ++i )) ; do
    if [ QA_${prj}.cpp = ${prj_cpp[i]} ] ; then
      makeProject $prj
      continue 2
    fi
  done

  undefPrj=${prj}
done

if [ "${undefPrj}" ] ; then
  echo "undefined project(s): ${undefPrj[*]}"
  exit 41
fi

exit 0
