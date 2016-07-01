.. _best-practice:

================
 Best Practice
================

Installation
============
* Fast and easy: conda install -c birdhouse -c conda-forge qa-dkrz
* Full sources: git clone https://github.com/IS-ENES-Data/QA-DKRZ
* Use access to locally provided libraries via the ``install_configure`` file
* Verify the success of the installation by running ``qa-dkrz --example[=path]``,
  see :ref:`results`.

Before a Run
============

* Apply default settings of the project tables
* Gather frequently changed options in a file and provided on the command-line (-f);
  this file could also contain QA_CONF=with-user-defined-name.
* Options on the command-line (but -f) only for testing or rechecking small data sets.
* Use the SELECT option to partition the entire data set, e.g. for CORDEX,
  ``SELECT AFR-44=`` (note that in this case '=' is required;
  may-be with prefixed '.*/' depending on PROJECT_DATA)
  would check the data of every model available for the specified domain.
  On the other hand, ``SELECT .*/historical=orog`` would find any orography
  file in all historical in the given ``PROJECT_DATA=sub-dir-tree``.
* Similarly to ``EXP_PATH_INDEX``, option ``PT_PATH_INDEX`` defines the name of
  a consistency table, which is created and utilised to check consistency across
  sub-atomic files and across the parent experiment, if available,
  of a given variable. This kind of table is consulted for
  experiments and versions matching the same name.

Operational Mode
================

* Before starting to check data, please make sure that the configuration was set
  as desired by ``qa-dkrz [opts] -e_show_conf``.
* Check the selected experiments: ``qa_DKRZ [opts] -e_show_exp``.
* Try for a single file first and inspect the log-file, i.e. run
  ``qa-dkrz [opts] -e_next``, then resume without -e_next.
* Use ``nohup`` for long-term execution in the background.
* If the script is run in the foreground, then command-line option '-m' may
  be helpful by showing the current file name and the number
  of variables (done/selected) on a status line .
* Have a look at the human readable QA results in directory
  ``QA_RESULTS/check_logs/Summary/experiment-name.``
* Manual termination of a session: if an immediate break is required,
  please inquire the process-id (pid), e.g. by ps -ef, and execute the
  command 'kill -TERM pid'. This will close the current session neatly
  leaving no remnants and ready for resumption.
