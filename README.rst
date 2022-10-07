====================================================================
QA-DKRZ - Quality Assurance Tool for Climate Data -- FPSCONV branch
====================================================================

|build-status| |conda-install|

:Version: 0.6.7-41c_conv

The Quality Assurance tool QA-DKRZ developed at DKRZ checks conformance
of meta-data of climate simulations given in `NetCDF` format with conventions
and rules of projects. At present, checking of CF Conventions, CORDEX, CMIP5, and
CMIP6, the latter with the option to run PrePARE (D. Nadeau, LLNL) is supported. The check results are summarised in json-formatted files.

THIS is the adaptation for the CORDEX FPS Convection project

Getting Started
===============

Prerequisites
-------------

Use the conda environment provided:

::

  conda env create -f qa-dkrz-env.yml

Installation
------------

::

  git clone https://github.com/doblerone/QA-DKRZ_FPSCONV.git
  cd QA-DKRZ_FPSCONV
  ./install --compile
  ./install --force CORDEX
  ./install --force CF

Test
----

Edit ``qa-test.task`` for your needs and:

::

  ./scripts/qa-dkrz -f qa-test.task


Documentation
=============

QA-DKRZ applies Sphinx, and the latest documentation can be found on
`ReadTheDocs`_.

.. _ReadTheDocs: http://qa-dkrz.readthedocs.org


Getting Help
============

T.b.d. (slack etc.)


Bug tracker
===========

T.b.d (issues trakcer?)

Contributing
============

The sources of `QA-DKRZ` are available on Github:

You are highly encouraged to participate in the development.

License
=======

t.b.d. 

