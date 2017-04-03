 1029  cd W_QA-DKRZ
 1030  cd conda-recipes/cmip6-cmor-tables/
 1034  more meta.yaml 
 1036  more build.sh 
 1040  cd .. qa-dkrz/
 1044  vi build.sh 
 1046  vi meta.yaml 
 1049  more qa-wrapper.sh
 1053  more meta.yaml 
 1061  conda build -c conda-forge -c defaults .
 1072  anaconda upload /hdh/local/miniconda/conda-bld/linux-64/qa-dkrz-0.6.5-1.tar.bz2
