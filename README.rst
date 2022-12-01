===================================================================
QA-DKRZ - Quality Assurance Tool for Climate Data -- FPSCONV branch
===================================================================

|build-status| |conda-install|

:Version: 0.6.7-41c_conv

The Quality Assurance tool QA-DKRZ developed at DKRZ checks conformance
of meta-data of climate simulations given in `netCDF` format with conventions
and rules of projects. At present, checking of CF Conventions, CORDEX, CMIP5, 
and CMIP6, the latter with the option to run PrePARE (D. Nadeau, LLNL) is 
supported. The check results are summarised in json-formatted files.

THIS is the adaptation of the AQ-DKRZ tool (a.k.a. QA-checker) for the CORDEX FPS 
Convection project.

Getting Started
===============

<<<<<<< HEAD
Prerequisites
-------------

Use the conda environment provided:

::

  conda env create -f qa-dkrz-env.yml
  conda activate qa-dkrz-env

Installation
------------

Within the previous environment:

::

  git clone https://github.com/doblerone/QA-DKRZ_FPSCONV.git
  cd QA-DKRZ_FPSCONV
  ./install --compile # Enter QA tables path when prompted 
  ./install --force CORDEX
  ./install --force CF

Test
----

Edit ``qa-test.task`` for your needs and:

::

  ./scripts/qa-dkrz -f qa-test.task
=======
Quick-start guide
-----------------
>>>>>>> main

This is a step-by-step guide to getting started in using the pre-installed 
QA-checker at the project-internal ``jsc-cordex`` data processing exchange 
infrastructure. This is not a documentation or HowTo guide for using the 
``jsc-cordex`` machine in general.

**1. Prerequisites**

You need a personal account on the ``jsc-cordex`` server.

For bash shell, set the FPSCONV root directory; this variable is just used 
in this manual for clarity (it is not needed to run the QA-checker)::

   export FPSCONV_ROOT=/mnt/CORDEX_FPS_CPCM

The QA-checker does not need any environment variables to be set.

**2. Install directory of the QA-checker**

The centrally installed and maintained production version of the QA-checker is 
under (based on this github repo)::

   $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV

The configuration tables, needed to adjust the QA-checker to specific 
experiments are in the their pmost current version here::

   $FPSCONV_ROOTS/Software/adobler/git/QA-DKRZ_FPSCONV/tables/projects/CORDEX-FPSCONV

**3. Preparation per user**

Has to be done only once, permission setting for using the installed version
and configuration of the individual QA-checker working directory, the 
``$FPSCONV_ROOT/tmp/$USER`` is the preferred temporary working directory for
the users::

   git config --global --add safe.directory $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV

   mkdir -p $FPSCONV_ROOT/tmp/$USER/qa-dkrz-check
   mkdir -p $FPSCONV_ROOT/tmp/$USER/QA/results

Now get the task file, which is runcontrol or namelist file for the QA-checker.
Copy the default, you may have as many of these files as needed, they contain 
the instructions which tasks the QA-checker is supposed to do this file will
be edited later::

   cd $FPSCONV_ROOT/tmp/$USER/qa-dkrz-check
   cp $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/qa-test.task .

The QA-checker works with tabulated information that describes what to check
and which results to expect; the QA-checker needs to know where to find these 
tables; one may either use the "official" tables that are provided with the 
checker (our default here) or use modified tables at alternative locations. Run
the following command which creates mandatory config file under 
``$HOME/.qa-dkrz/config.txt`` whith the location of the tables; when run for 
the first time or when no config file is found it asks for the location of the
QA-tables::

   ./$FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/install 

This is the content of the ``$HOME/.qa-dkrz/config.txt`` and for a first test
leave this unchanged (now without the ``$FPSCONV_ROOT`` nomenclature::

   /mnt/CORDEX_FPS_CPCM/Software/adobler/git/QA-DKRZ_FPSCONV:
   QA_TABLES=/mnt/CORDEX_FPS_CPCM/Software/adobler/git/QA-DKRZ_FPSCONV/tables
   UPDATE=frozen

**4. Run the QA-checker**

In the working directory adjust the ``task``-file, it contains several 
examples on how it might be modified::

   cd $FPSCONV_ROOT/tmp/kgoergen/qa-dkrz-check
   vim qa-test.task

If you want to run the QA-checker with a defaultconfiguration in terms of range
checking etc., then the most important / only things to adjust are the 
following variables in ``qa-test.task``.

The path to the CMORized netCDF files to be checked, on ``jsc-cordex`` 
(replace ``$USER`` and ``$FPSCONV_ROOT`` accordingly); the checker goes through
directoryi hierarchy recursively::

   PROJECT_DATA=$FPSCONV_ROOT/CORDEX-FPSCONV/output/ALP-3/FZJ-IDL/SMHI-EC-Earth/rcp85

The results of the check, this is highly structured output::

   QA_RESULTS=$FPSCONV_ROOT/tmp/$USER/QA/results

Which variables and and which time interval to check; many more options are
possible; here: check hourly data, check everything the QA-checker recursivly
finds under ``$PROJECT_DATA``::

   SELECT .*/1hr/*

Finally run the QA-checker::

   cd $FPSCONV_ROOT/tmp/kgoergen/qa-dkrz-check
   ./$FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/scripts/qa-dkrz -f qa-test.task

On a daily basis mainly Step 4 and 5 will be repeated. It may be useful to have
multiple task files, dependent on the temporal rersolution and experiment to 
check.

**5. Results**

The outcome of the checking, via ASCII log file, can be found here, keep the
logs, they might be needed as proof later on::

$FPSCONV_ROOT/tmp/$USER/QA/results/check_logs/FZJ-IDL_SMHI-EC-EARTH_historical_fpsconv-x1n2-v1_r12i1p1_1hr.log

**6. Performance**

To run the QA-checker concurrently (x4) on several netCDF files and / or variables,
set this in the ``qa-test.task``::

   NUM_EXEC_THREADS=4

**7. Custom QA-tables**

Customized tables in addition / combination with the official tables might help to
capture real issues with the data which may just be ignored as warnings with the
default tables otherwise.

Documentation
=============

For the CORDEX FPSCONV implementation, this ``README`` file is the primary 
documentation.

QA-DKRZ applies Sphinx, and the latest documentation can be found on
`ReadTheDocs`_.

.. _ReadTheDocs: http://qa-dkrz.readthedocs.org

Getting Help
============

T.b.d. (slack, etc.)

Bug tracker
===========

T.b.d (issues tracker?)

Contributing
============

The sources of `QA-DKRZ` are available on Github: https://github.com/h-dh/QA-DKRZ

You are highly encouraged to participate in the development.

License
=======

t.b.d.
