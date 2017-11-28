.. _overview:

========
Overview
========

What is checked?
================

The Quality Assurance (QA) tool developed at DKRZ tests the compliance
of meta-data of climate simulations given in `NetCDF` format to conventions and
rules of projects. Additionally, the QA checks time sequences
and the technical state of data (i.e. occurrences of `Inf` or `NaN`, gaps,
replicated sub-sets, etc.) for atomic data set entities, i.e. variable and
frequency, e.g. `tas` and `mon`
for monthly means of near-surface air temperature`. When atomic data sets
are subdivided into several files, then changes between these files in
terms of (auxiliary) coordinate variables will be detected as well as gaps or
overlapping time ranges. This may also apply to follow-up experiments.

At present, the QA checks data of the projects `CMIP5`, `CMIP6` and `CORDEX`
by consulting tables based on requested controlled vocabulary and requirements.
When such pre-defined information is given about directory structures,
format of file names, variables and attributes, geographical domain, etc.,
then any deviation from compliance will be annotated in the QA results.

Files with meta-data similar to a project, but a few deviating features from the default
tables can also be checked. User-defined tables may disable particular checks
and supersede e.g. the default project name.

While former versions took for granted that meta-data of files were valid in
terms of the NetCDF Climate and Forecast (CF) Meta Data Conventions,
compliance is now verified. The CF check is both embedded in the
QA tool itself and provided by a stand-alone tool, which is described below.
Also available is a test suite used during the development phase.

The development of a QA-DKRZ CMOR3 check module was frozen with
the 2016-ESGF_F2F_Conference (http://esgf.llnl.gov). Since then, the CMOR3 checker ``PrePARE`` is applied for checking CMIP6 files
(http://cmor.llnl.gov/mydoc_cmip6_validator).
The output by `PrePARE` is merged into the flow of QA-DKRZ annotations.

After installation, QA-DKRZ runs are started on the command-line.

.. code-block:: bash

   $ qa-dkrz [command-line opts, -f task-file, config-opts] [file.nc]

- command-line opts: usually for non-operational usage; mostly prefixed by ``--``
- \- \-help displays description of command-line options.
- task-file: a container with config-options for specific settings
- config-opts: given in project specific files, e.g. CORDEX_qa.conf; provides a default. Every option of PROJECT_qa.conf can be provided on the command-line with the prefix ``-e|E[ |_]option``, e.g. -e next. Note that option names are case-insensitive while values are not.
- file.nc: only for a quick test.


Available Versions
==================

The QA-DKRZ package is a rolling release, however tagged.
