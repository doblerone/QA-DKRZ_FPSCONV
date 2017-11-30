=================================================
QA-DKRZ - Quality Assurance Tool for Climate Data
=================================================

|build-status| |conda-install|

:Version: 0.6.7

The Quality Assurance tool QA-DKRZ developed at DKRZ checks conformance
of meta-data of climate simulations given in `NetCDF` format with conventions
and rules of projects. At present, checking of CF Conventions, CORDEX, CMIP5, and
CMIP6, the latter with the option to run PrePARE (D. Nadeau, LLNL) is supported. The check results are summarised in json-formatted files.


Getting Started
===============

The recommended way to install QA-DKRZ is by the conda package manager.
External tables are downloaded at run-time, when needed for
dedicated projects.

.. code-block:: bash

   $ conda create -n qa-dkrz -c conda-forge -c h-dh qa-dkrz

.. note:: *CMIP6*: The development of the internal CMOR checker of QA-DKRZ
          is frozen. Instead, ``PrePARE`` (https://cmor.llnl.gov)
          is run by ``qa-dkrz``. The output of ``PrePARE`` is merged into the flow of QA-DKRZ annotations.

          .. code-block:: bash

             $ conda create -n cmor -c conda-forge -c pcmdi cmor

There are different ways to access the QA-DKRZ checker ``qa-dkrz``:

- ``source [conda-path/]activate qa-dkrz`` and run with ``qa-dkrz``
- ``path/miniconda/envs/qa-dkrz/bin/qa-dkrz``
- ``alias qa-dkrz=path/miniconda/envs/qa-dkrz/bin/qa-dkrz``

If a machine is not suited for Linux-64Bits or you would like to work
with the sources, then see :ref:`installation` for details.

The success of the installation may be checked by running ``qa-dkrz`` for a single
NetCDF file. Provision of a project name, e.g. `-P CORDEX`, is required in this simple case. Initially, you'll be asked for a path to QA-TABLES (to put it simply,  a directory for external tables; details in :ref:`installation`).

.. code-block:: bash

   $ qa-dkrz -P PROJECT-Name file.nc

The QA-DKRZ module for checking CF Conventions is also available stand-alone:

.. code-block:: bash

   $ dkrz-cf-checker [ops] file.nc

Running the QA_DKRZ tool requires external tables which are not provided by conda packages. ``qa-dkrz`` delegates installation and updates to a script, when
the first argument is ``install`` .

.. code-block:: bash

   $ qa-dkrz install [opts] PROJECT-Name


Documentation
=============

QA-DKRZ applies Sphinx, and the latest documentation can be found on
`ReadTheDocs`_.

.. _ReadTheDocs: http://qa-dkrz.readthedocs.org


Getting Help
============

Please, direct questions or comments to hollweg@dkrz.de

Mailing list
------------

Join the mailing list ...


Bug tracker
===========

If you have any suggestions, bug reports or annoyances, please, report
to our issue tracker at https://github.com/IS-ENES-Data/QA-DKRZ/issues

Contributing
============

The sources of `QA-DKRZ` are available on Github:
https://github.com/IS-ENES-Data/QA-DKRZ

You are highly encouraged to participate in the development.

License
=======

Please notice the *Disclaimer of Warranty* (`DoW.txt`) file in the top distribution
directory.

.. |build-status| image:: https://travis-ci.org/IS-ENES-Data/QA-DKRZ.svg?branch=master
   :target: https://travis-ci.org/IS-ENES-Data/QA-DKRZ
.. |conda-install| image:: https://anaconda.org/birdhouse/qa-dkrz/badges/installer/conda.svg
   :target: https://anaconda.org/birdhouse/qa-dkrz
