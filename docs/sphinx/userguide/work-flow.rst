.. _work-flow:

=========
Work-flow
=========

Please note that the work-flow has changed. By now, operational usage is
generally distinct from installation or updating. However, users may trigger
that tables are updated regularly (by default daily). But, conda built packages
are updated only manually. The work-flow is generally as:

**Installation/update**

- download the QA-DKRZ package (conda or GitHub)
- CMIP6: download the CMOR3 package (both conda and GitHub)
- run ``qa-dkrz install [opts] PROJECT`` : download external tables (both conda and HitHub), compilation of executables (only GitHub)
- file HOME/.qa-dkrz/config.txt is created

The paths to the ``qa-dkrz install`` script (bash) is:

*conda-built*
   path: miniconda/env/qa-dkrz/opt/qa-dkrz/install

*GitHub*
   path: QA-DKRZ/install

Paths and times of next updates
are kept in a configuration file located in `HOME/.qa-dkrz/config.txt`
by default. It is not necessary for a user to edit it, but permitted. It ismanaged by ``qa-dkrz install`` calls.

**Operation**

The names of the operational executables are different between conda-built
and GitHub-based packages, because of backward-compatibility. However, all
work (almost) the same.

.. code-block:: bash

  *conda-built*
   qa-dkrz:    python script, path= miniconda/env/qa-dkrz/bin
   qa-dkrz.sh: bash script,   path= miniconda/env/qa-dkrz/bin

  *GitHub*
   qa-dkrz.py: python script, path= QA-DKRZ/python/qa-dkrz/qa-dkrz.py
   qa-dkrz:    bash script,   path= QA-DKRZ/scripts/qa-dkrz

Operation is conducted (with some reasonable options) by e.g.

.. code-block:: bash

  $ qa-dkrz -f task-file

with task-file containing frequently modified or task-specific options like
``PROJECT_DATA``, ``QA_RESULTS``, ``SELECT``, ``EMAIL_SUMMARY``, ``NUM_EXEC_THREADS``,
``CHECK_MODE`` and ``PROJECT`` or ``QA_CONF``.
The full set of options used by default is given and explained in file
``QA_DKRZ/tables/projects/PROJECT/PROJECT_qa.conf``
with ``QA_DKRZ`` indicating the root of the QA-DKRZ package.
Additionally, every valid option
may be provided on the command-line by prefixing ``-e|E[ ]``.

Note that option ``RUN_CMOR3_LLNL`` must be given for running `PrePARE`,
however is set for CMIP6 by default .

The separation of operation and install/updating is superseded by option ``--auto-up``
for backward-compatibility and convenience, respectively.

A corrupt or incomplete installation becoming apparent during an operational run
would issue a message for resolving the conflict.
