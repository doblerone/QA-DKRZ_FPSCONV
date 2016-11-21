# Makefile for the QA package

# Cx   executables of stand-alone C   program
# CPPx executables of stand-alone C++ program

Cx    =  diskUsage.x fModTime.x  unixTime.x
CPPx  =  check_CORDEX_standard-table.x convert_CF-standard-names.x \
         getNC_att.x getStatus.x syncFiles.x testValidNC.x

UTIL_SOURCE = hdhC.cpp ReadLine.cpp Split.cpp Statistics.cpp GetOpt_hdh.cpp
UTIL_HEADER = hdhC.h matrix_array.h readline.h split.h statistics.h getopt_hdh.h

BASE_SOURCE = Annotation.cpp Base.cpp BraceOP.cpp Date.cpp \
   InFile.cpp NcAPI.cpp TimeControl.cpp Variable.cpp
BASE_HEADER = annotation.h base.h brace_op.h data_statistics.h date.h \
   geo_meta.h in_file.h iobj.h matrix_array.h nc_api.h \
   time_control.h variable.h

QA_SOURCE = CF.cpp CellStatistics.cpp FD_interface.cpp Oper.cpp OutFile.cpp Parse.cpp \
    QA.cpp QA_data.cpp QA_DRS_CV_Table.cpp QA_time.cpp QA_cnsty.cpp QA_main.cpp
QA_HEADER = cell_statistics.h cf.h fd_interface.h oper.h out_file.h parse.h \
    qa.h qa_data.h qa_drs_cv_table.h qa_cnsty.h qa_main.h qa_time.h

#vpath %.c   $(QA_SRC)/src
#vpath %.cpp $(QA_SRC)/src
#vpath %.h   $(QA_SRC)/include

VPATH = $(QA_SRC)/src $(QA_SRC)/include
all:    c-prog cpp-prog $(PRJ_NAME)

c-prog: $(Cx)

diskUsage.x: diskUsage.c
fModTime.x: fModTime.c
unixTime.x: unixTime.c

%.x: %.c
	$(CC) $(CFLAGS) -o $@ $<

cpp-prog: $(CPPx)

getNC_att.x: $(UTIL_SOURCE) $(UTIL_HEADER) brace_op.h BraceOP.cpp nc_api.h \
     annotation.h Annotation.cpp NcAPI.cpp getNC_att.cpp
getStatus.x: $(UTIL_SOURCE) $(UTIL_HEADER) getStatus.cpp
syncFiles.x: $(UTIL_SOURCE) $(UTIL_HEADER) date.h Date.cpp \
     nc_api.h NcAPI.cpp syncFiles.cpp
testValidNC.x: $(UTIL_SOURCE) $(UTIL_HEADER) nc_api.h NcAPI.cpp testValidNC.cpp

%.x: %.cpp
	$(CXX) $(CXXFLAGS) -o $@  $< \
        -I $(QA_SRC)/include $(INCLUDE) $(LIB) $(LIBDL) $(GCC_LIBS)

           #-ludunits2 -lnetcdf -lhdf5_hl -lhdf5 -lz -luuid

${PRJ_NAME}: ${PRJ_NAME}.x

${PRJ_NAME}.x: ${BASE_SOURCE} ${BASE_HEADER} $(QA_PRJ_HEADER) $(QA_PRJ_SRC) \
               $(QA_SOURCE) $(QA_HEADER) $(UTIL_SOURCE) $(UTIL_HEADER)
	$(CXX) $(CXXFLAGS) -o ${PRJ_NAME}.x $(QA_SRC)/src/QA_main.cpp \
           -I $(QA_SRC)/include $(INCLUDE) \
           $(LIB) $(LIBDL) $(GCC_LIBS)

           #-ludunits2 -lnetcdf #-lhdf5_hl -lhdf5 -lz \
           #-lmfhdf -ldf -ljpeg -lssl -lcrypto

#          -DREVISION=$(REVISION) \

CF-checker: dkrz-cf-checker.x

dkrz-cf-checker.x: CF_main.cpp CF.cpp $(BASE_SOURCE) ${BASE_SOURCE} \
                   $(QA_PRJ_HEADER) $(QA_HEADER) $(UTIL_SOURCE) $(UTIL_HEADER)
	$(CXX) $(CXXFLAGS) -o dkrz-cf-checker.x $(QA_SRC)/src/CF_main.cpp \
           -I $(QA_SRC)/include $(INCLUDE) \
           $(LIB) $(LIBDL) $(GCC_LIBS)

           #-ludunits2 -lnetcdf #-lhdf5_hl -lhdf5 -lz \
           #-lmfhdf -ldf -ljpeg -lssl -lcrypto
