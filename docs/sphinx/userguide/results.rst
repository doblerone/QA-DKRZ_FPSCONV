.. _results:

=======
Results
=======

Running ``qa-dkrz`` generates QA results with a few directories.

**check_logs**
  All results are gathered in this directory.

  Besides the log-files containing the check results for all files, summarised results are given in two modes:

  * JSON formatted files and information about atomic time ranges of variables
    (used by default since 2017).

  * Before then, a less machine readable format, which is still used when new results are written to a directory where files of this format are already present. Also, when option TRADITIONAL_SUMMARY is applied.

  **Log-files** (files: experiment-name.log, YAML format)
    Files with applied QA-DKRZ options as preamble followed by
    entries for every checked file; possibly with annotations. The file name
    is composed corresponding to options DRS_PATH_BASE and LOG_PATH_INDEX,
    respectively LOG_FILE_INDEX; see see :ref:`configuration`.

  **Annotations** (files: experiment-name.json)
    Experiment-wide information about check conclusion, DRS structure and
    annotations if any. Each annotation refers DRS path components, and severeness levels (if any).

  **Period** (files: experiment-name.period in YAML format, **time_range.txt**)
    The time range for each variable. Shorter ranges are marked.

  **Summary** (at present only for qa-dkrz.sh)

    **experiment-names**
      Directories: experiment-name, contained files are human readable)

      **annotations.txt**
        - inform about missing variables, if a project defines a set of required  ones.
        - file names with a time-range stamp that violates project rules, e.g.  overlapping with other files, syntax failure, etc..
        - In fact copies of the tag-files describe below.

      **tag-files**
        a file for each annotation flag found in the corresponding log-file.
        Annotations include path, file name and variable or experiment scope,
        respectively, caption, impact level,
        and the tag from one of the check-list tables.

**cs_table** (only when option CHECKSUM is enabled)
  Check sums of files; for the verification that no old file is sold as new.

**data**
  internal usage

**session_logs**
  internal usage

**tables**
  The tables actually used for the run and if option PT_PATH_INDEX is set,
  then also for the consistency table generated during a check.
