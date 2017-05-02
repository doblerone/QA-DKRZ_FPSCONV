.. _best-practice:

================
 Best Practice
================

Installation
============

* Quick and easy: ``conda create -n qa-dkrz -c conda-forge -c h-dh qa-dkrz``
* Full sources: ``git clone https://github.com/IS-ENES-Data/QA-DKRZ``

  * Use access to locally provided libraries via the ``install_configure`` file

* Verify the success of the installation by running ``qa-dkrz --example[=path]``,
  see :ref:`results`.
* CMIP6 with PrePARE.py: ``conda create -n cmor -c conda-forge -c pcmdi -c uvcdat cmor``
* Automatic updating is enabled by option ``--auto-up`` until ``--auto-up=d[isable]`` is provided;
  works for both ``qa-dkrz`` and ``install``.

Before a Run
============

* There is a default for almost every option in the project tables
* Gather frequently changed options in a file and provided on the command-line (-f);
  this file could also contain QA_CONF=with-user-defined-name.
* Options on the command-line (but -f) only for testing or rechecking small data sets.
* Use the SELECT option to partition the entire data set, e.g. for CORDEX,
  ``SELECT AFR-44=`` (note that in this case '=' is required;
  Without regExp '.*/' as prefix, the first path component must follow the last one
  of option ``PROJECT_DATA``, i.e. the base-path to project data.)
  Example: ``SELECT .*/historical=orog`` would find any orography
  file in all historical sub-directories starting at ``PROJECT_DATA``.
* Option  provides a comma-separated list of indices defining


* The following two names are defined by a comma-separated list of indices
  pointing to the DRS path components. Counting begins with ``0`` at the positions
  indicated by options ``DRS_PATH_BASE=path-component``.

  * ``LOG_PATH_INDEX`` for the names of log-files
  * ``CT_PATH_INDEX`` for the names of so-called consistency tables
    created and utilised to check consistency across sub-atomic files and
    across parent experiments of a given variable. This kind of table is consulted
    for experiments and versions matching the same name.

Operational Mode
================

* Before starting to check data, please make sure that the configuration was set
  as desired by running ``qa-dkrz [opts] -e_show_conf``.
* Check the selected experiments: ``qa_DKRZ [opts] -e_show_exp``.
* Try for a single file first and inspect the log-file, i.e. run
  ``qa-dkrz [opts] -e_next``. If all right, then resume without -e_next.
* Use ``nohup`` for long-term execution in the background.
* If the script is run in the foreground, then command-line option '-m' may
  be helpful by showing the current file name and the number
  of variables (done/selected) on a status line .
* Have a look at the QA results in directory ``QA_RESULTS/check_logs``; annotations
  are summarised in sub-directory ``Annotationi/experiment-name.json`` .
* Manual termination of a session: if an immediate break is required,
  please inquire the process-id (pid), e.g. by ``ps -ef | grep qa-dkrz``,
  and execute the command 'kill -TERM pid'. This will close the current session
  neatly leaving no remnants and being ready for resumption.
