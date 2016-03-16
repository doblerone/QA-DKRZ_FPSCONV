

build docker image from Dockerfile::

    $ docker build -t centos6-build .

run docker image::

    $ docker run --rm -it -v /hdh/hdh/QA-DKRZ/:/home/hdh/QA-DKRZ:rw centos6-build
