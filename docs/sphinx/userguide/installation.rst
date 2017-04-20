.. _installation:

============
Installation
============

`QA-DKRZ` may be installed  either via the `conda` package manager or from a GitHub repository.

Recommended is the conda package manager installing
a ready-to-use package. At present, a 64-bit processor is required.

If you want to work with sources or use a different machine architecture, then the
installation from source should be the first choice.

For CMIP6, `QA-DKRZ` binds the CMOR validator `PrePARE.py` (D. Nadeau, LLNL,
https://cmor.llnl.gov). This is conducted only by `conda`, e.g.

.. code-block:: bash

  $ conda create -n cmor -c conda-forge -c pcmdi -c uvcdat cmor

Details in section Work-flow below.

No matter what the preference is `conda` or from `GitHub`,
you are asked for a location of tables and programs during the installation.


.. _conda-install:

By Conda Package Manager
========================

Make sure that you have
`conda <http://conda.pydata.org/docs/install/quick.html#linux-miniconda-install>`_ installed.
The quick steps to install `miniconda` on Linux 64-bit are:

.. code-block:: bash

   $ wget https://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh
   $ bash Miniconda-latest-Linux-x86_64.sh

.. note:: The installation is tested on various 64-bit machines.

Please check that the ``conda`` package manager is in your ``PATH``.
For example you may set the ``PATH`` environment variable as following:

.. code-block:: bash

    $ export PATH=$PATH:your-path/your-miniconda/bin
    $ conda -h

`conda` provides the QA-DKRZ package with all dependencies included.
You may install it to the root environment or to any other with a name of your choice, e.g.

.. code-block:: bash

   $ conda create -n qa-dkrz -c conda-forge -c h-dh qa-dkrz


From GitHUB repository
======================


Requirements
------------

The tool requires the BASH shell and a C/C++ compiler (works for AIX, too).


Building the QA tool
--------------------

The sources are downloaded from GitHub by

.. code-block:: bash

   $ git clone  https://github.com/IS-ENES-Data/QA-DKRZ

Any installation is done with the script ``install`` ( a prefix './' could
be helpful in some cases) within the ``QA-DKRZ`` root directory. Please, have also a look
at the Work-flow section.

- A file ``install_configure`` in ``QA-DKRZ`` binds
  necessary ``lib`` and ``include`` directories. A template ``.install_configure``
  is provided. If not copied to ``install_configure`` and edited before the first launch,
  then it is created and the user is asked to edit the file.
- Environmental variables CC, CXX, CFLAGS, and CXXFLAGS are accepted.
- ``install`` establishes access to libraries, which may be linked (recommended) or built
  (triggered by option ``--build``).
- Proceedings are logged in file ``install.log``.
- The executables are compiled for projects named on the command-line.
- When ``install`` is started the first time for a given project, then corresponding
  external tables and programs are downloaded to a ``QA_HOME`` directory;
  the user is asked for the path.
- The user's home directory contains a config.txt file in directory .qa-dkrz
  created and updated by user defined control statements.

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


.. _work-flow:

=========
Work-flow
=========

Please note that the work-flow has changed. By now, operational mode is
generally distinct from installation or updating.

Operation is conducted (with some reasonable options) by e.g.

.. code-block:: bash

  $ qa-dkrz -f task-file

with task-file containing frequently or task-specific options like
``PROJECT_DATA``, ``QA_RESULTS``, ``SELECT``, ``EMAIL_SUMMARY``, ``NUM_EXEC_THREADS``,
``CHECK_MODE`` and ``PROJECT`` or ``QA_CONF``.
The full set of options used by default is given in file
`QA_HOME/tables/projects/PROJECT/PROJECT_qa.conf`
with ``QA_HOME`` chosen during the installation. Additionally, every valid option
may be provided on the command-line by prefixing ``-e``.

Note that option ``RUN_CMOR3_LLNL`` must be given for running `PrePARE.py`.

Installation and update tasks are indicated by option ``--install=...``, which
is additionally provided to the `qa-dkrz` call. Then, all options of `install`
have to be given by a comma-separated list; the usual ``--`` prefix of options
may be omitted, e.g.

.. code-block:: bash

  $ qa-dkrz --install=help

would display all install options.
The ``--install`` option may be merged with operational options conducted afterwards.

The separation of operation and install/updating is broken by option ``--auto-up`` for
backward-compatibility. Additionally, when working with the GitHub based package,
the ``--install`` option may be replaced by the `QA-DKRZ/install` command
(then options with ``--`` prefix).
Note that `conda` does not know the path to `install`.

Operation/installation/update with conda requires `activating` and `deactivating` the
corresponding environment. Provided that `conda` is within the path or aliased:

.. code-block:: bash

  $ source activate your-chosen-environment-name
  $ qa-dkrz options
  $ source deactivate.

Command `which qa-dkrz` while the environment is activated reveals the path to qa-dkrz.
Given to `PATH` or as `alias` would make activate/deactivate redundant.

There are three stages for updating:

*install [PROJECT]* without any additional option.
  Compilation of executables for PROJECT. Only for GitHub based installation; no
  effect for a conda installed `qa-dkrz`.

*install --up*
  Updating once every other number of days, while num has been given by option ``--uf=num``
  or one day be default.

*install --force*
  Unconditional update

Information is kept in a configuration file located in `~/.qa-dkrz/config.txt` by default.
User may not edit it, but could. If missing, it is restored during the next `qa-dkrz` call.

A corrupt or incomplete installation becoming apparent during an operational run would issue
a message for resolving the conflict.

For those who want to have full access to the sources, but do not like to grapple with
library dependencies, a hybrid operation of conda and GitHub based sources QA_DKRZ is possible.
Just bind the conda installed libs to the file `QA-DKRZ/install_configure` and operate/update
from the GitHub based QA-DKRZ.

