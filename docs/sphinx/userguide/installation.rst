.. _installation:

============
Installation
============

`QA-DKRZ` may be installed  either by the ``conda`` package manager or
from a GitHub repository. Recommended is ``conda`` installing
a ready-to-use package (a 64-bit processor is required). If you want to work with sources or use a different machine architecture, then the
installation from GitHUb should be the first choice.

Running the QA_DKRZ tool requires external tables which are not provided by conda packages (neither by QA_DKRZ from GitHub). ``qa-dkrz`` starts a script for installation and updates, when the first argument is ``install`` .

For CMIP6, `QA-DKRZ` binds the CMOR validator `PrePARE` (D. Nadeau, LLNL,
https://cmor.llnl.gov). The CMOR package is downloaded by

.. code-block:: bash

  $ conda create -n cmor -c conda-forge -c pcmdi cmor

Details in section Work-flow below.

No matter what the preference is `conda` or `git`, operational
and update processes are separated.

.. note::
         - ``qa-dkrz:`` just for runs. While the completeness of required tables is checked, but the state whether tables / programs are up-to-date is not. If this check fails, the user is asked to run the command below.

         - ``qa-dkrz install [opts] PROJECT:`` get/update external tables and, when installed from GitHub, also compile executables. If run initially, you are asked for a path to put external tables. The user is disengaged from knowing table names or their internet access.


Conda Package Manager
=====================

Make sure that you have a working conda environment.
The quick steps to install `miniconda` on Linux 64-bit are:

.. code-block:: bash

   $ search and download Miniconda-latest-Linux-x86_64.sh
   $ bash Miniconda-latest-Linux-x86_64.sh

Have the ``conda`` in your ``PATH`` or execute it by ``path/miniconda/bin/conda``.
`Conda` provides the QA-DKRZ package with all dependencies included.
It is recommended to use conda's default for named environments found in ``path/miniconda/envs`` .

.. code-block:: bash

   $ conda create -n qa-dkrz -c conda-forge -c h-dh qa-dkrz


GitHUB repository
=================

Requirements
------------

The tool requires the BASH shell and a C/C++ compiler (works for AIX, too).


Building the QA tool
--------------------

The sources are downloaded from GitHub by

.. code-block:: bash

   $ git clone  https://github.com/IS-ENES-Data/QA-DKRZ

Any installation is done with the script ``install`` ( a prefix './' could
be helpful in some cases) within the ``QA-DKRZ`` root directory.
Please, have also a look at the Work-flow section.

- A file ``install_configure`` in ``QA-DKRZ`` binds
  necessary ``lib`` and ``include`` directories. If ``install_configure`` is not available, a template is created and the user is asked to edit the file.
- Environmental variables CC, CXX, CFLAGS, and CXXFLAGS are accepted.
- ``qa-dkrz install`` establishes access to libraries, which may be linked (recommended) or built
  (triggered by option ``--build``. Note that this is rarely tested of the courwse of development).
- Proceedings are logged in file ``install.log``.
- The executables are compiled for projects named on the command-line.
- If ``qa-dkrz install`` is started the first time for a given project, then
  corresponding external tables and programs are downloaded to directory
  `QA_TABLES`; the user is asked for the path.
- The user's home directory contains a config.txt file in directory .qa-dkrz
  by default, created and updated by ``qa-dkrz install``.

The full set of options is described by:

.. code-block:: bash

  $ qa-dkrz install --help

Building Libraries
------------------

If you decide to use your own set of libraries (accessing provided ones
is preferred by respective settings in the install_configure file), then
this is accomplished by

.. code-block:: bash

  $ qa-dkrz install --build [opts]

Sources of the following libraries are downloaded and installed:

- zlib: www.zlib.net,
- hdf5: www.hdfgroup.org,
- netcdf-4: www.unidata.ucar.edu (shared, no FORTRAN, non-parallel),
- udunits: http://www.unidata.ucar.edu/packages/udunits.
- uuid: mostly provided by the operating system.

The libraries are built in sub-directory ``local/source``.
If libraries had been built previously, then the sources are updated and
the libraries are rebuilt.
