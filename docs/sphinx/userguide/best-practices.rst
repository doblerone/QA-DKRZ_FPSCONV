.. _best-practices:

===============
 Best Practices
===============

------------
Installation
------------

* ``conda create -n qa-dkrz -c conda-forge -c h-dh qa-dkrz``
*  CMIP6 with PrePARE: ``conda create -n cmor -c conda-forge -c pcmdi cmor``
*  run ``path/miniconda/env/qa-dkrz/opt/install`` for downloading/updating required external tables; not when frozen.

---------
Operation
---------

* run ``qa-dkrz -f task-file`` with the following options contained in a task_file:

  - PROJECT_DATA
      path to the root of the data tree.
  - QA_RESULTS
      path to results (created by qa-dkrz) containing log information.
  - DRS_PATH_BASE
      directory name indicating the beginning of the DRS sub-path within the data path. May be omitted, when it equals the name of the project in capital letters. If your data path does not match a DRS pattern, set also LOG_PATH_INDEX and LOG_FILE_INDEX depending on your requirements. This will determine the name of the log-file in QA_RESULTS/check_logs.
  - SELECT
      partitioning of a data directory-tree. Usually starting with the next one behind PROJECT_DATA; regular expressions enabled.
  - NUM_EXEC_THREADS
      number of asynchronous, parallel proccesses; each for an atomic variable.
  - EMAIL_SUMMARY=name@site.xy
      a summary of the results are emailed.
  - QA_CONF=PROJECT_qa.conf
      default setting of options for a given project.

* Before starting to check data, please make sure that the configuration was set
  as expected by running ``qa-dkrz [opts] -e_show_conf``.
* Check the selected experiments: ``qa_DKRZ [opts] -e_show_exp``.
* Try for a single file first and inspect the log-file, i.e. run
  ``qa-dkrz [opts -f task-file] -e_next``. If accepted, then resume without ``-e_next``.
* Files having checked before are skipped when they are scheduled again for a check. Use option -e_clear=note for rechecking files causing annotations.
* Use ``nohup`` for long-term execution in the background.
* If the script is run in the foreground, then command-line option '-m' may
  be helpful by showing the current file name that .
* Have a look at the QA results in directory ``QA_RESULTS/check_logs``; annotations
  are summarised in sub-directory ``Annotation/experiment-name.json`` .
* Manual termination of a session: if an immediate break is required,
  please inquire the process-id (pid), e.g. by ``ps -ef | grep qa-dkrz``,
  and execute the command 'kill -TERM pid'. This will close the current session
  neatly leaving no remnants and being ready for resumption.

----------------
General remarks:
----------------


- Projects taken into account are CORDEX, CMIP5/6, HAPPI.
- There is a proceeding with tables:
  \
    * QA-DKRZ based tables, which are taken by default, reside in ``QA-DKRZ/tables/projects/PROJECT``. Note that changes would be lost after an update, regardless of whether by ``conda`` or ``git``.
    * the default tables as well as external tables are copied to the QA-TABLES path given in HOME/.qa-dkrz/config.txt or asked initially.
    * user-define adjustments have to be placed in QA-TABLES/tables/PROJECT, where PROJECT_DRS_CSV.csv has to be copied entirely and adjusted, while PROJECT_check-list.conf and PROJECT_qa.conf may only contain adjusted statements from the respective default table.
    * all tables required for a particular run are copied to QA_RESULTS/tables.  Depending on the QA_RESULT managment, these may be overwriten when changed. The checking program will read from here.
- If the recommendations above are sufficient for checking your project, you may skipp reading from here.
- If you would like to check something similar to supported project, then you may try option PROJECT_AS and copy/adjust main-project tables to the user-defined tables.
- If the machine you're on does not have the ability to run the code, then you would like to download from GitHub and compile the executables.
- QA-DKRZ may be used in multi-user and multi-tasking mode. If everybody should use the same configuration, then ``install --freeze`` would be an option. Otherwise, each user may have his own ``HOME/.qa-dkrz/config.txt`` and external tables.
