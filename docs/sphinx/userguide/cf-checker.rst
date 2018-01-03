.. _cf-checker:

======================
CF Conventions Checker
======================

The CF Conformance checker applies to conventions 1.4 -1.7 (cfconventions.org).

The checker is a bash script and resides in

**conda-built**
   path=``miniconda/env/qa-dkrz/bin/dkrz-cf-checker.sh``

**GitHub**
   path=``QA-DKRZ/scripts/dkrz-cf-checker``

.. code-block:: bash

    dkrz-cf-checker [opts] files(s)

    Purpose: Check for CF Conventions Compliance (http://cfconventions.org).
    The checker is part of the QA-DKRZ package (https://github.com/IS-ENES-DATA)
    Compilation: '/your-path-to-QA-DKRZ/install CF' unless
    the package was downloaded via 'conda create -n qa-dkrz  -c conda-forge -c h-dh qa-dkrz'.
      -C str     CF Convention string; taken from global attributes by default.
      -F path    Find recursively all nc-files from starting point 'path'.
      -p str     Path to one or more NetCDF files.
      -R         Apply also recommendations given by the CF conventions.
      --debug    Show execution commands.
      --help
      --param    Only for program development.
      --ts[=arg] Run the files provided in the Test-Suite for CF Conventions
                 rules in QA-DKRZ/CF_TestSuite. If particular NetCDF files are
                 provided additionally, then only these are used. If a filename
                 cannot be resolved unambiguously, then use optional arg F[ail] or P[ass}.


CF Test Suite
=============

A collection of NetCDF files designed to cover all rules of the CF Conventions
derived from the examples of the cf-conventions-1.6.pdf documents. There are
two branches.

   PASS
      Proper meta-data and data sets, which should never output a failure.
      Number of files: 81

   FAIL
      Each file contains a (mostly) single break of a rule and should never pass the test.
      Number of files: 187

The suite is checked entirely by applying option ``--ts``. When filenames are
additionally passed to the checker, then only these are checked. If such
filenames are without a leading path, the checker tries to find out the location
in the two branches. If it should be available in both, then provide P or F
as argument to ``--ts``.

Please, note that running some files from the suite by a different
checker may raise additional annotations, because the files are in fact only
snippets with a partly missing data segment.
