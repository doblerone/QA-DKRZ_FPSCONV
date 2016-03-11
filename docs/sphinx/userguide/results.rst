.. _results:

=======
Results
=======

Running ``qa-dkrz --example[=path]``, where omission of ``path`` would do
this in the current directory, generates QA results;
actually after some preparations, the command
``qa-dkrz -m -f qa-test.task --work=path`` is executed.
Note that option ``--work`` only creates the ``config.txt`` file and the
``table`` directory in the directory pointed to by the option;
the user's home directory is the location by default.
Option ``-m`` displays some information on a status line.

In more detail, there are (for experiment-name see :ref:`configuration`):

**check_logs**
  All results are gathered in this directory.

  **Log-files** (experiment-name.log, YAML format)
    There is an entry for every checked file; possibly with annotations.
    The option setting is prepended.

  **Annotations** (experiment-name.note, YAML format)
    Only those entries from the log-files with annotations. The entries for
    all files of given variable-frequency are merged
    to experiment and variable scope if possible.

  **Period** (experiment-name.period, YAML format)
    The time range for each variable. If shorter than found for others, then
    a flag is present.

  **Summary**
    Directories (experiment-name, contained files are human readable)

    **experiment-name**
      **annotations.txt**

      * inform about missing variables, if a project defines a set of required
        ones.
      * file names with a time-range stamp that violates project rules, e.g.
        overlapping with other files, syntax failure, etc. .
      * In fact copies of the tag-files describe below.

      **time_range.txt**
        tables of atomic time-ranges spanned by the variables for each frequency.
        Shorter atomic ranges or non-continuous sub-temporal ranges are marked.

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
