=================================================
QA-DKRZ - Quality Assurance Tool for Climate Data
=================================================

|build-status| |conda-install|

:Version: 0.5.x

The Quality Assurance tool QA-DKRZ developed at DKRZ checks conformance
of meta-data of climate simulations given in `NetCDF` format with conventions
and rules of projects. At present, checking of CF Conventions, CORDEX, CMIP5, and
CMIP6, the latter with the option to run PrePARE.py (D. Nadeau, LLNL,
https://cmor.llnl.gov) is supported. The check results are summarised in json-formatted files.

.. note:: The QA tool is still in a testing stage.

Getting Started
===============

The recommendated way to install QA-DKRZ is to use the conda package manager.

.. code-block:: bash

   $ conda create -n qa-dkrz -c conda-forge -c h-dh qa-dkrz

If a machine is not suited for Linux-64Bits or you would like to work
with the sources, then see :ref:`installation`
for details.

The success of the installation may be checked either by running

.. code-block:: bash

   $ qa-dkrz --example

(creating a directory `example`
in the current path. More details in :ref:`results`)
or by operating the stand-alone-checker with a NetCDF file of your choice.

.. code-block:: bash

   $ dkrz-cf-checker your-choice.nc


Documentation
=============

QA-DKRZ is using Sphinx, and the latest documentation can be found on
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

If you have any suggestions, bug reports or annoyances please report them
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
