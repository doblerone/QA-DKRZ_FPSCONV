#!/bin/bash
SCRIPT=$(cd ${0%/*} && echo $PWD/${0##*/})
SCRIPTPATH=`dirname ${SCRIPT}`

export QA_PATH=`cd "${SCRIPTPATH}/../opt/qa-dkrz" && pwd -P`

exec python ${QA_PATH}/python/qa-dkrz/qa-dkrz.py $*
