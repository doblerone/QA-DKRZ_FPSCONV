FROM centos:6
MAINTAINER https://github.com/IS-ENES-Data/QA-DKRZ

LABEL Description="Centos 6 build image to build qa-dkrz" Vendor="DKRZ" Version="0.1.0"

# Prepare system
RUN yum update -y && yum install -y gcc g++ git vim wget

# Add user hdh
RUN useradd -d /home/hdh -m -u 1000 -g users hdh

# Remaining tasks run as user hdh
USER hdh

# Change to user home
WORKDIR /home/hdh

# Install miniconda
RUN wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O miniconda.sh
RUN bash miniconda.sh -b -p /home/hdh/miniconda

# Set env
ENV HOME /home/hdh
ENV PATH /home/hdh/miniconda/bin:$PATH

# Update conda
RUN conda update -y -q conda

# Add additional conda channels
RUN conda config --add channels birdhouse && \
    conda config --add channels ioos

# Install qa-dkrz and other useful packages
RUN conda install -y -c birdhouse -c ioos qa-dkrz

# Start with bash session
CMD ["/bin/bash"]


