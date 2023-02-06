#/bin/bash                                                                                                   
set -e -x 

# Download miniconda3
wget -O ${SCRATCH}/miniconda.sh https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh 
bash ${SCRATCH}/miniconda.sh -b -p ${SCRATCH}/miniconda3

# Configure Conda
conda config --set always_yes yes --set changeps1 no
conda config --add channels conda-forge 

# Initialize conda
source ${SCRATCH}/miniconda3/etc/profile.d/conda.sh

# Install environment
conda env create -f environment.yml

# Remove download file
rm -f ${SCRATCH}/miniconda.sh
