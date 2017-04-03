#!/bin/bash -eu
CC="$PREFIX/bin/gcc"
CXX="${PREFIX}/bin/g++"
LIBPATH="${PREFIX}/lib"
LDFLAGS="-Wl,-rpath ${LIBPATH}"

echo "CC=${CC}" > install_configure
echo "CXX=${CXX}" >> install_configure
echo "CFLAGS=\"-Wall\"" >> install_configure
echo "CXXFLAGS=\"-Wall -std=c++11 -D NC4\"" >> install_configure
echo "LIB=${LIBPATH}" >> install_configure
echo "INCLUDE=\"${PREFIX}/include\"" >> install_configure

# prepare qa home in opt/qa-dkrz
QA_SRC=${PREFIX}/opt/qa-dkrz
mkdir -vp ${QA_SRC}
cp -r ./scripts ${QA_SRC}
cp -r ./tables ${QA_SRC}
cp -r ./CF-TestSuite ${QA_SRC}
cp -r ./example ${QA_SRC}
cp -r ./src ${QA_SRC}
cp -r ./include ${QA_SRC}
cp README* ${QA_SRC}
cp Makefile ${QA_SRC}
cp install_configure ${QA_SRC}
touch ${QA_SRC}/.ignore_GitHub # avoids git update!

# run build
#export QA_PATH="$PWD"
touch .ignore_GitHub # avoids git update!
export QA_LIBS="-ludunits2 -lnetcdf -lhdf5_hl -lhdf5 -lz -luuid -lmfhdf -ldf -ljpeg -lssl -lcrypto"
QA_HOME=/hdh/hdh/Test/HOME
./install --net=f --lcf --qa-src=$QA_SRC --qa-home="$QA_HOME" CF CMIP5 CMIP6 CORDEX

# copy generated files to opt/qa-dkrz
#cp -r ./bin ${QA_SRC}
cp ./example/templates/qa-test.task ${QA_SRC}
#cp install* ${QA_SRC}
#cp .install_configure ${QA_SRC}

# write git version to install.log
echo "branch=$(git branch | grep '*' | awk '{print $2}')" > ${QA_SRC}/install.log
echo "hexa=$(git log --pretty=format:'%h' -n 1)" >> ${QA_SRC}/install.log

# install wrapper script in bin/ to call cfchecker and qa-dkrz
cp $RECIPE_DIR/cfchecker-wrapper.sh $PREFIX/bin/dkrz-cf-checker
cp $RECIPE_DIR/qa-wrapper_env.sh $PREFIX/bin/qa-dkrz

chmod +x $PREFIX/bin

# put into conda's root/bin
pref=${PREFIX%/*} # strip bin
pref=${pref%/*}   # strip qa-dkrz

if [ ${pref##*/} = envs ] ; then
  cp $RECIPE_DIR/qa-wrapper_rbin.sh ${pref%/*}/bin
  chmod +x ${pref%/*}/bin
fi

