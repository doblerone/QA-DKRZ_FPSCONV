===================================================================
QA-DKRZ - Quality Assurance Tool for Climate Data -- FPSCONV branch
===================================================================

|build-status| |conda-install|

:Version: 0.6.7-41c_conv

The Quality Assurance tool QA-DKRZ developed at DKRZ checks data and conformance
of meta-data of climate simulations given in `netCDF` format with conventions
and rules of projects. At present, checking of CF Conventions, CORDEX, CMIP5, 
and CMIP6, the latter with the option to run PrePARE (D. Nadeau, LLNL) is 
supported. The check results are summarised in json-formatted files.

THIS is the adaptation of the QA-DKRZ tool (a.k.a. QA-checker) for the CORDEX FPS 
Convection project.

General installation using your own conda environment
===============

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

Quick-start guide at FZJ
=================

This is a step-by-step guide to getting started in using the pre-installed 
QA-checker at the project-internal ``jsc-cordex`` data processing exchange 
infrastructure. This is not a documentation or HowTo guide for using the 
``jsc-cordex`` machine in general.

**1. Prerequisites**

You need a personal account on the ``jsc-cordex`` server.

For bash shell, set the ``$FPSCONV_ROOT`` directory; this variable is just used 
in this manual for clarity (it is not needed to run the QA-checker)::

   export FPSCONV_ROOT=/mnt/CORDEX_FPS_CPCM

The QA-checker does not need any environment variables to be set.

**2. Install directory of the QA-checker**

The centrally installed and maintained production version of the QA-checker 
(based on this github repo) is under::

   jsc-cordex:$FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV

The configuration tables, needed to adjust the QA-checker to specific 
experiments are in the their most current version here::

   jsc-cordex:$FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/tables/projects/CORDEX-FPSCONV

More on how to adjust these tables to the user needs for checking individual
experiments is documented below.

**3. Preparation per user**

This preparation has to be done only once. 

Refine permission setting for using the installed version::

   git config --global --add safe.directory $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV

Setup your individual QA-checker working directory::

   mkdir $FPSCONV_ROOT/tmp/$USER

Create diretories for the control script and the results of the QA-checker::

   mkdir -p $FPSCONV_ROOT/tmp/$USER/qa-dkrz-check
   mkdir -p $FPSCONV_ROOT/tmp/$USER/QA/results

Now get the task file, which is a runcontrol or namelist file for the QA-checker.
Copy the default, you may have as many of these files as needed, they contain 
the instructions, which tasks the QA-checker is supposed to do. This file will
be edited later::

   cd $FPSCONV_ROOT/tmp/$USER/qa-dkrz-check
   cp $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/qa-test.task .

The QA-checker works with tabulated information that describes what to check
and which results to expect (see below on how to customize these); the 
QA-checker needs to know where to find these tables; one may either use the 
"official" tables that are provided with the checker (our default here) or use 
modified tables at an alternative directory. 

The following command creates a mandatory config file under 
``$HOME/.qa-dkrz/config.txt`` whith the location of those tables; when run for 
the first time or when no config file is found it asks for the location of the
QA-tables::

   ./$FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/install

This is the content of the ``$HOME/.qa-dkrz/config.txt`` and for a first test
leave this unchanged (now without the ``$FPSCONV_ROOT`` nomenclature::

   /mnt/CORDEX_FPS_CPCM/Software/adobler/git/QA-DKRZ_FPSCONV:
   QA_TABLES=/mnt/CORDEX_FPS_CPCM/Software/adobler/git/QA-DKRZ_FPSCONV/tables
   UPDATE=frozen

**4. Set-up the QA-checker task**

In the working directory adjust the ``task``-file, it contains several 
examples on how it might be modified::

   cd $FPSCONV_ROOT/tmp/$USER/qa-dkrz-check
   vim qa-test.task

If you want to run the QA-checker with a default configuration in terms of range
checking etc., then the most important / only things to adjust in 
``qa-test.task`` are the following variables:

1. The path to the CMORized netCDF files to be checked, on ``jsc-cordex`` 
(replace ``$USER`` and ``$FPSCONV_ROOT`` accordingly); the checker goes through
directory hierarchy recursively::

   PROJECT_DATA=$FPSCONV_ROOT/CORDEX-FPSCONV/output/ALP-3/FZJ/C-Earth/evaluation

``$FPSCONV_ROOT/CORDEX-FPSCONV/output/`` is usually where your data will reside.

2. The path to the results of the check (this is highly structured output in 
several subdirectories)::

   QA_RESULTS=$FPSCONV_ROOT/tmp/$USER/QA/results

3. Which variables and and which time interval to check; many options are
possible. In order to check all vars and all files in the hourly data directory
recursively set::

   SELECT .*/1hr/*

If you want to have everything checked recursivly, just set like this; the 
results for the temporal aggregations will be nicely ordered in the 
``QA_RESULTS`` directory::

   SELECT=.*/

If you wwant to check only tas and pr in the hourly files for starters::

   SELECT .*/1hr/tas,.*/1hr/pr

When using the tool for different experiments (e.g. your evaluation run vs the 
historical run, etc.), only the ``PROJECT_DATA`` and ``QA_RESULTS`` essentially 
need adjustment.

**5. Run the QA-checker**

Finally, to run it::

   cd $FPSCONV_ROOT/tmp/$USER/qa-dkrz-check
   bash $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/scripts/qa-dkrz -f qa-test.task

or, put it in the background, with default ``nohup.out`` log file::

   cd $FPSCONV_ROOT/tmp/$USER/qa-dkrz-check
   nohup time bash $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/scripts/qa-dkrz -f qa-test.task &

On a daily basis mainly Step 4 and 5 will be repeated. 

It may be useful to have multiple task files, dependent on the temporal 
resolution and experiment to check. You can find a set of templates for that 
in::

   $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/example/templates/

*There are likely many issues when you run the first test as most likely some
adjustments to the QA-chekcer lookup tables might be needed.*

**Important:** The QA-checker needs Python 2.7; an installation exists on
``jsc-cordex`` under ``/mnt/CORDEX_FPS_CPCM/Software/adobler/miniconda2`` and 
the system-wide call to python refers to this install. If you need Python 3 for 
any of your tasks on jscr-cordex, invoke with ``python3``.

**6. Results**

The outcome of the checking, via ASCII log files, can be found in the 
``QA_RESULT`` directory you defined in the task file, e.g.::

   $FPSCONV_ROOT/tmp/$USER/QA/results/check_logs/FZJ-IDL_SMHI-EC-EARTH_historical_fpsconv-x1n2-v1_r12i1p1_1hr.log

For testing the checker, it is useful to move, remove or have a unique name 
for the results folder or even a larger, more differentiated directory depth
each time you start the checker. Otherwise the reusults may be confusing.

The results directory also contains the lookup tables and the config files used
during the checking, as a means to document exactly how the QA-checker was run.

A good place to start investigating the outcome of the QA-checker check is to 
look into the ``check_logs/`` directory and its contents, which are
self-explanatory.

*Once you enter the final checking stage however, keep the logs, they might be 
needed or at least useful as proof later on.*

**7. Performance**

To run the QA-checker concurrently (x4) on several netCDF files and / or 
variables, set this anywhere the ``qa-test.task``::

   NUM_EXEC_THREADS=4

In some application the checker ran with 4 threads on a pretty complete 
CORDEX-FPSCONV CMORized output with all variables, temporal aggregation levels 
and all checks with 15 years of data about 2.5h on jsc-cordex.

**8. Custom QA-tables**

Customized tables in addition / combination with the official tables might help 
to capture real issues with the data, which may just be ignored as warnings 
with the default tables otherwise.

What this means is that you most likley need to adjust the default tables to 
avoid trivial error messages, so that the real issues stand out.

The default tables are located here::

   $FPSCONV_ROOT/Software/adobler/git/QA-DKRZ_FPSCONV/tables/projects/CORDEX-FPSCONV

If you want your own tables, do as follows.

- Create an alternative table-directory, e.g. here: ``$FPSCONV_ROOT/tmp/$USER/qa_custom_tables``

- In ``$HOME/.qa-dkrz/config.txt`` modify the path in ``QA_TABLES`` to this new table-directory.

- You do not need to do anything (I think), when running for the first time, the CORDEX-FPSCONV tables from the directory above are copied (or you copy them  beforehand yourself).

- What would you edit? Maybe the "model_id" in ``CORDEX_RCMs_ToU.txt`` you have been using is not yet in the tables (e.g., WRF381BB). Or the driving GCM is not yet contained in ``CORDEX_GCMModelName.txt``. Just edit those tables and others according to your needs.

- **BUT BE AWARE:** You can only change or add things within a certain limits! What is not registered with the ESGF data nodes in the controlled vocabulary you cannot yust add here as the ESGF data node will reject this data. So what is not listed here https://github.com/ESGF/config/blob/master/publisher-configs/ini/esg.cordex-fpsconv.ini cannot be changed in the tables of the CORDEX-FPSCONV. *This ini file was adjusted in cooperation with ESGF people a while ago.*

Side-remark: In the task file ``PROJECT="CORDEX"`` is mentioned, this is needed  for some technical work-around; in fact in the software-directory above, CORDEX points to CORDEX-FPSCONV with some modified tables.

Documentation
=============

For the CORDEX FPSCONV implementation, this ``README.rst`` file is the primary 
documentation.

QA-DKRZ applies Sphinx, and the latest documentation can be found on
`ReadTheDocs`_.

.. _ReadTheDocs: http://qa-dkrz.readthedocs.org

Getting Help
============

Feel free to use the slack channel: https://app.slack.com/client/T01FXMXLE4S/C01FR0ENXC6

Bug tracker
===========

Please use the issue tracker here on github: https://github.com/doblerone/QA-DKRZ_FPSCONV/issues

Contributing
============

The sources of `QA-DKRZ` are available on Github: https://github.com/h-dh/QA-DKRZ

You are highly encouraged to participate in the development.

License
=======

For research purposes only.
