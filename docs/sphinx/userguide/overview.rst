.. _overview:

========
Overview
========

What is checked?
================

The Quality Assurance (QA) tool developed at DKRZ tests the conformance
of meta-data of climate simulations given in `NetCDF` format to conventions and
rules of projects. Additionally, the QA checks time sequences
and the technical state of data (i.e. occurrences of `Inf` or `NaN`, gaps,
replicated sub-sets, etc.) for atomic data set entities, i.e. variable and
frequency, e.g. `tas` and `mon`
for monthly means of near-surface air temperature`. When atomic data sets
are subdivided into several files, then changes between these files in
terms of (auxiliary) coordinate variables will be detected as well as gaps or
overlapping time ranges. This may also apply to follow-up experiments.

At present, the QA checks data of the projects `CMIP5` and `CORDEX`
by consulting tables based on requested controlled vocabulary and requirements.
When such pre-defined information is given about directory structures,
format of file names, variables and attributes, geographical domain, etc.,
then any deviation from compliance will be annotated in the QA results.

While former versions took for granted that meta-data of files were valid in
terms of the NetCDF Climate and Forecast (CF) Meta Data Conventions,
the conformance is now verified. The CF check is both embedded in the
QA tool itself and provided by a stand-alone tool, which is described below.
Also available is a test suite used during the development phase.

After installation, QA-DKRZ runs are started on the command-line.

.. code-block:: bash

   $ /package-path/scripts/qa-DKRZ [opts]

.. note:: The name of the tool has changed over the years as well as the name of
          the starting script. At first, the acronym QC (Quality Control/Check)
          was used, but was changed to the more commonly used term
          QA (Quality Assurance). The former script names qcManager as well as
          qc_DKRZ, and the underscore replaced by a hyphen work, too.
          Also, option names containing QC or qc are still
          valid for backward compatibility.


Available Versions
==================

At present, the QA-DKRZ package is a rolling release
and is available on GitHub

.. code-block:: bash

   $ git clone  https://github.com/IS-ENES-Data/QA-DKRZ

