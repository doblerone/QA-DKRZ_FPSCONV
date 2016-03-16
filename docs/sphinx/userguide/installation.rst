.. _installation:

============
Installation
============

`QA-DKRZ` may be installed  either via the conda package manager or from source.

The conda package manager installs a ready-to-use package, however without any
sources. Also, 64-bit machines are required.

If you want to have sources or use a different machine architecture, then the
installation from source should be first choice.


.. _conda-install:

By Conda Package Manager
========================

Make sure that you have
`conda <http://conda.pydata.org/docs/install/quick.html#linux-miniconda-install>`_ installed. The quick steps to install `miniconda` on Linux 64-bit are:

.. code-block:: bash

   $ wget https://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh
   $ bash Miniconda-latest-Linux-x86_64.sh

.. note:: The installation is tested on 64-bit Centos 6 and
          Ubuntu/Debian 14.04 LTS.

Please check that the ``conda`` package manager is on your ``PATH``.
For example you may set the ``PATH`` environment variable as following:

.. code-block:: bash

    $ export PATH=~/miniconda/bin:$PATH
    $ conda -h

The QA-DKRZ package is available on the
`birdhouse anaconda channel <https://anaconda.org/birdhouse/qa-dkrz>`_, which
is installed by running the following command (all dependencies are included):

.. code-block:: bash

   $ conda install -c birdhouse -c ioos qa-dkrz


From Source
===========


Requirements
------------

The tool requires the BASH shell and a C/C++ compiler (AIX works, too).


Building the QA tool
--------------------

The sources are downloaded from GitHub by

.. code-block:: bash

   $ git clone  https://github.com/IS-ENES-Data/QA-DKRZ

Any installation is done with the script ``install`` ( a prefix './' could
be helpful in some cases).

- By default, a config.txt file and tables of various projects are
  copied to .qa-dkrz in the users home-directory.
- The script stops by asking for editing a file ``install_configure``, which
  will be protected against any update.
- Environmental variables CC, CXX, CFLAGS, and CXXFLAGS are accepted.
- ``install`` establishes access to libraries, which may be linked or built,
  as well as updating sources and executables.
- Option ``--build`` triggers building of libraries.
- Proceedings are logged in file ``install.log``.

The full set of options is described by:

.. code-block:: bash

  $ ./install --help

Building Libraries
------------------

If you decide to use your own set of libraries (accessing provided ones
is preferred by respective settings in the install_configure file), then
this is accomplished by

.. code-block:: bash

  $ ./install --build [opts]

Sources of the following libraries are downloaded and installed:

- zlib-1.2.8 from www.zlib.net,
- hdf5-1.8.9 from www.hdfgroup.org,
- netcdf-4.3.0 from www.unidata.ucar.edu (shared, no FORTRAN, non-parallel),
- udunits package from http://www.unidata.ucar.edu/packages/udunits.

The libraries are built in sub-directory ``local/source``.
If libraries had been built previously, then the sources are updated and
the libraries are rebuilt.

.. _updates:

=======
Updates
=======

Updating the QA sources from the repository and re-compilation of executables
is done automatically by default for both kinds of installation. This may be
switched off by

.. code-block:: bash

  $ /package-path/install --auto-up=disable

and reversed by ``--auto-up``. In particular for the installation from sources,
i.e. using the ``git`` tool, the creation of an empty file ``.ignore_GitHub``
in the QA-DKRZ path disables updating of the sources, too.

Analogous for tables of projects: ``--auto-table-up``.

.. note:: If enabled, then every qa-dkrz run triggers the install-tool
          for a search of updates of QA-DKRZ itself,
          while updating of project tables is done only once a day.

