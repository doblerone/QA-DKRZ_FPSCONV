'''
Created on 21.03.2016


@author: hdh
'''

import sys
import os
import shutil
import subprocess
import re

import qa_util

def cpTables(key, table, table_path, qaOpts, pDir):
    prj=qaOpts.getOpt('PROJECT_AS')
    qaHome = qaOpts.getOpt('QA_HOME')
    pwd= os.getcwd()

    if len(pDir) == 0:
        # regular precedence of paths, i.e. highest first
        if os.path.isfile(table):
            # absolute path
            pDir.append(table)
        else:
            # local file
            if os.path.isfile( os.path.join(pwd, table) ):
                pDir.append(os.path.join(pwd, table))

        if qaOpts.isOpt('USE_STRICT'):
            # USE_STRICT: only the project's default directory
            pDir.append(os.path.join('tables', 'projects', prj) )
        else:
            pDir.append('tables')
            pDir.append(os.path.join('tables', prj) )
            pDir.append(os.path.join('tables', 'projects') )
            pDir.append(os.path.join('tables', 'projects', prj) )

    if prj != 'CF':
        if table[0:3] == 'CF_' or table[0:3] == 'cf-':
            pDir.append( os.path.join('tables', 'projects', 'CF') )

    if key.find('CHECK_LIST') > -1:
        # concatenate existing files
        toCat=[]
        for pD in pDir:
            t = os.path.join(qaHome, pD, table)
            if os.path.isfile(t):
                toCat.append(t)

        # is any of the files newer than the destination?
        dest = os.path.join(table_path, table)
        dest_modTime = qa_util.f_get_mod_time(dest)
        for f in toCat:
            if qa_util.f_get_mod_time(f) > dest_modTime:
                qa_util.cat(toCat, dest)
                break
    else:
        # just copy the file with highest precedence; there should only
        # be a single one for each kind of table
        for ix in range(len(pDir)):
            src = os.path.join(qaHome, pDir[ix], table)
            if os.path.isfile(src):
                dest= os.path.join(table_path, table)
                if qa_util.f_get_mod_time(src) > qa_util.f_get_mod_time(dest):
                    shutil.copyfile(src,dest)
                    break

    return


def init_session(g_vars, qaOpts):
    g_vars.curr_date = qa_util.date()
    g_vars.session   = g_vars.curr_date
    g_vars.session_logdir = os.path.join(g_vars.res_dir_path,
                               'session_logs', g_vars.curr_date)

    qaOpts.setOpt('SESSION', g_vars.session)
    qaOpts.setOpt('SESSION_LOGDIR', g_vars.session_logdir)

    if qaOpts.isOpt('SHOW'):
        return

    qa_util.mkdirP(g_vars.session_logdir) # error --> exit

    with open(os.path.join(g_vars.session_logdir,
                            'pid.' + g_vars.pid), 'w') as fd:
        fd.write( os.getcwd() + '\n')
        for a in sys.argv:
            fd.write(' ' + a)
        fd.write(' --fpid ' + str(g_vars.pid) + '\n')

    return


def init_tables(g_vars, qaOpts):
    TP='TABLE_PATH'

    g_vars.table_path = qaOpts.getOpt(TP)
    tp_sz = len(g_vars.table_path)

    if tp_sz:
        # user-defined
        if g_vars.table_path[tp_sz-1] == '/':
            g_vars.table_path = g_vars.table_path[:tp_sz-1]
    else:
        g_vars.table_path = os.path.join(g_vars.res_dir_path, 'tables')

    qa_util.mkdirP(g_vars.table_path)
    qaOpts.setOpt(TP, g_vars.table_path)

    # Precedence of path search for tables:
    #
    #   tables/projects/${PROJECT}
    #   tables/projects
    #   tables/${PROJECT}
    #   tables

    # 1) default tables are provided in QA_SRC/tables/projects/PROJECT.
    # 2) a table of the same name provided in QA_SRC/tables gets
    #    priority, thus may be persistently modified.
    # 3) tables from 2) or 1) are copied to ${QA_Results}/tables.
    # 4) Option TABLE_AUTO_UPDATE is the default, i.e. tables in
    #    projects are updated and are applied.
    # 5) 4) also for option USE_STRICT.

    if not os.path.isdir(g_vars.table_path):
        qa_util.mkdirP(g_vars.table_path)

    # collect all table names in a list
    tables={}

    for key in qaOpts.dOpts.keys():
        tName = ''

        # project tables
        if key.find('TABLE') > -1:
            tName = qaOpts.getOpt(key)
        elif key.find('CHECK_LIST') > -1:
            tName = qaOpts.getOpt(key)
        elif key.find('CF_') > -1 and key[3] != 'F':
            tName = qaOpts.getOpt(key)

        if len(tName):
            regExp = re.match(r'^[a-zA-Z0-9\._-]*$', tName)
            if regExp:
                tables[key] = tName

    qaOpts.setOpt('TABLES', tables)

    pDir=[]
    for key in tables.keys():
        cpTables( key, tables[key], g_vars.table_path, qaOpts, pDir)

    return


def run_install(qaOpts, g_vars):
    # update external tables and in case of running qa_dkrz.py from
    # sources update C++ executables

    if not qaOpts.isOpt('PROJECT'):
        qaOpts.setOpt('PROJECT', 'NONE')
    if not qaOpts.isOpt('PROJECT_AS'):
        qaOpts.setOpt('PROJECT_AS', qaOpts.getOpt('PROJECT'))

    if not g_vars.NO_GIT:
        # checksum of the current qa_dkrz.py
        md5_0 = qa_util.get_md5sum(sys.argv[0])

        p = os.path.join(g_vars.qa_src, 'install')
        if qaOpts.isOpt('install_args'):
            p += ' ' + qaOpts.getOpt('install_args')

        try:
            subprocess.check_call(p, shell=True)
        except:
            return

        md5_1 = qa_util.get_md5sum(sys.argv[0])

        if md5_0 == md5_1:
            # restart the script
            pass

    return


def run(log, g_vars, qaOpts, rawCfgPars, cfgFile):
    #g_vars.TTY = os.ttyname(0)

    # are git and wget available?
    g_vars.NO_GIT=True
    if qa_util.which('git'):
        g_vars.NO_GIT = False

    g_vars.NO_WGET=True
    if qa_util.which('wget'):
        g_vars.NO_WGET = False

    # update external tables and in case of running qa_dkrz.py from
    # sources update C++ executables
    run_install(qaOpts, g_vars)

    if qaOpts.isOpt('NUM_EXEC_THREADS'):
        g_vars.thread_num = \
            sum( qa_util.mk_list(qaOpts.getOpt('NUM_EXEC_THREADS')) )
    else:
        g_vars.thread_num = 1

    g_vars.res_dir_path = qaOpts.getOpt('QA_RESULTS')
    g_vars.project_data_path = qaOpts.getOpt('PROJECT_DATA')
    g_vars.prj_dp_len = len(g_vars.project_data_path)

    init_session(g_vars, qaOpts)

    g_vars.check_logs_path = os.path.join(g_vars.res_dir_path, 'check_logs')

    g_vars.cs_enable = False
    if qaOpts.isOpt('CHECKSUM'):
        g_vars.cs_enable = True
        if qaOpts.isOpt('CHECKSUM', True):
            g_vars.cs_type = 'md5'
        else:
            g_vars.cs_type = qaOpts.getOpt('CHECKSUM')

        cs_dir = qaOpts.getOpt('CS_DIR')
        if len(cs_dir) == 0:
            cs_dir='cs_table'
        g_vars.cs_dir = os.path.join(g_vars.res_dir_path, cs_dir)

    qaOpts.setOpt('LOG_FNAME_DIR', g_vars.check_logs_path)
    qa_util.mkdirP(g_vars.check_logs_path) # error --> exit

    qa_util.mkdirP(os.path.join(g_vars.res_dir_path, 'data')) # error --> exit

    # some more settings
    if not qaOpts.isOpt('ZOMBIE_LIMIT'):
        qaOpts.setOpt('ZOMBIE_LIMIT', 3600)

    if not qaOpts.isOpt('CHECKSUM'):
        if qaOpts.isOpt('CS_STAND_ALONE') or qaOpts.isOpt('CS_DIR'):
            qaOpts.setOpt('CHECKSUM', True)

    # save current version id to the cfg-file
    qv=qaOpts.getOpt('QA_REVISION')
    if len(qv) == 0:
        qv=qa_util.get_curr_revision(g_vars.qa_src, g_vars.isConda)

    qv = qa_util.cfg_parser(rawCfgPars, cfgFile,
                           section=g_vars.qa_src, key='QA_REVISION', value=qv)

    qaOpts.setOpt('QA_REVISION', qv)
    g_vars.qa_revision = qv

    # table path and copy of tables for operational runs
    init_tables(g_vars, qaOpts)

    # unique exp_name and table_names are defined by indices of path components
    qa_util.get_experiment_name(g_vars, qaOpts, isInit=True)
    qa_util.get_project_table_name(g_vars, qaOpts, isInit=True)

    # enable clearance of logfile entries by the CLEAR option
    if qaOpts.isOpt('CLEAR_LOGFILE'):
        g_vars.clear_logfile = True
    else:
        g_vars.clear_logfile = False

    g_vars.ignore_temp_files = qaOpts.isOpt('IGNORE_TEMP_FILES')
    g_vars.syncFilePrg = os.path.join(g_vars.qa_src, 'bin', 'syncFiles.x')
    g_vars.checkPrg = os.path.join(g_vars.qa_src, 'bin',
                                     'qA-' + qaOpts.getOpt('PROJECT_AS') + '.x')

    if not os.access(g_vars.syncFilePrg, os.X_OK):
        print g_vars.syncFilePrg + ' is not executable'
        sys.exit(1)
    if not os.access(g_vars.checkPrg, os.X_OK):
        print g_vars.checkPrg + ' is not executable'
        sys.exit(1)

    g_vars.anyProgress = False

    return
