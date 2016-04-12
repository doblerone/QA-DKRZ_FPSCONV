'''
Created on 17.03.2016

@author: hdh
'''

import sys
import os
import shutil
import subprocess
import ConfigParser

import qa_util

from qa_config import QaConfig

(isCONDA, QA_SRC) = qa_util.getQA_SRC(sys.argv[0])

# for options on the command-line as well as in configuration files
qaOpts=QaConfig(QA_SRC)

if isCONDA:
    qaOpts.setOpt('CONDA', True)


# traditional config file, usually in the users' home directory
rawCfgPars = ConfigParser.RawConfigParser()

def final():
    qa_util.cfgParser(rawCfgPars, cfgFile, final=True)

    return

def init():
    # always defined
    qaResults = qaOpts.getOpt('QA_RESULTS')

    initSession(qaResults)

    checkLogs = os.path.join(qaResults, 'check_logs')

    qaOpts.setOpt('EXP_LOGDIR', checkLogs)
    qa_util.mkdirP(checkLogs) # error --> exit

    qa_util.mkdirP( os.path.join(qaResults, 'data') ) # error --> exit

    # are git and wget available?
    NO_GIT=True
    if qa_util.which('git'):
        NO_GIT=False

    NO_WGET=True
    if qa_util.which('wget'):
        NO_WGET=False

    # some more settings
    if not qaOpts.isOpt('ZOMBIE_LIMIT'):
        qaOpts.setOpt('ZOMBIE_LIMIT', 3600)

    if not qaOpts.isOpt('CHECKSUM'):
        if qaOpts.isOpt('CS_STAND_ALONE') or qaOpts.isOpt('CS_DIR'):
            qaOpts.setOpt('CHECKSUM', True)

    maxNumExecThreads=0
    if qaOpts.isOpt('NUM_EXEC_THREADS'):
        maxNumExecThreads = sum(qaOpts.getOpt('NUM_EXEC_THREADS'))

    # save current version id to the cfg-file
    qv=qaOpts.getOpt('QA_REVISION')
    if len(qv) == 0:
        qv=qa_util.getCurrRevision(QA_SRC, isCONDA)

    qv = qa_util.cfgParser(rawCfgPars, cfgFile,
                           section=QA_SRC, key='QA_REVISION', value=qv)

    qaOpts.setOpt('QA_REVISION', qv)

    # table path and copy of tables for operational runs
    initTables()

    return


def initProjectTableName():

    ptp = qaOpts.getOpt('PROJECT_TABLE_PREFIX')
    ptp_sz=len(ptp)

    if ptp_sz:
        if ptp[ptp_sz-1] != '_':
            ptp += '_'

    pt = qaOpts.getOpt('PROJECT_TABLE')
    pt_sz=len(pt)

    if pt_sz == 0:
        # generate a default name
        if not ptp_sz:
            ptp='pt_'

        if not qaOpts.isOpt('PT_PATH_INDEX'):
            qaOpts.setOpt('PROJECT_TABLE', qaOpts.getOpt('PROJECT_TABLE'))

    qaOpts.setOpt('PROJECT_TABLE_PREFIX', ptp)
    qaOpts.setOpt('PRJCT_BASENAME', ptp + pt)

    return


def initSession(qaResults):
    for key in [ 'SHOW_CONF', 'SHOW_CALL', 'SHOW_NEXT' ]:
        if key in qaOpts.dOpts:
            return

    curr_date = qa_util.date()
    session=os.path.join(qaResults, 'session_logs', curr_date)

    qa_util.mkdirP(session) # error --> exit
    qaOpts.setOpt('SESSION', session)

    return


def initTables():
    TP='TABLE_PATH'

    runTablePath=qaOpts.getOpt(TP)
    tp_sz = len(runTablePath)

    if tp_sz:
        # user-defined
        if runTablePath[tp_sz-1] == '/':
            runTablePath=runTablePath[:tp_sz-1]
    else:
        runTablePath = os.path.join(qaOpts.getOpt('QA_RESULTS'), 'tables')

    qaOpts.setOpt('TABLE_PATH', runTablePath)
    qa_util.mkdirP(runTablePath)

    pt_path_index=qaOpts.getOpt('PT_PATH_INDEX')

    if len(pt_path_index):
        pt_path_index_max =  max(pt_path_index)
    else:
        pt_path_index_max=100

    qa_util.mkdirP(runTablePath)
    qaOpts.setOpt(TP, runTablePath)

    # Precedence of path search for tables:
    #
    #   tables/projects/${PROJECT}
    #   tables/projects
    #   tables/${PROJECT}
    #   tables

    # 1) default tables are provided in ${QA_SRC}/tables/projects/PROJECT.
    # 2) a table of the same name provided in ${QA_SRC}/tables gets
    #    priority, thus may be persistently modified.
    # 3) tables from 2) or 1) are copied to ${QA_Results}/tables.
    # 4) Option TABLE_AUTO_UPDATE is the default, i.e. tables in
    #    projects are updated and are applied.
    # 5) 4) also for option USE_STRICT.

    # collect all table names in a list
    tables={}

    for key in qaOpts.dOpts.keys():
        # project tables
        if len(key) > 5 and key[0:5] == 'TABLE':
            if key != 'TABLE_PATH':
                tables[key] = qaOpts.getOpt(key)

        # CF tables
        elif len(key) > 2 and key[0:3] == 'CF_':
            t = qaOpts.getOpt(key)
            tExtension=t[len(t)-4:]
            if tExtension == '.txt' or tExtension == '.xml':
                tables[key] = t


    qaOpts.setOpt('TABLES', tables)

    for key in tables.keys():
        initT( key, tables[key], runTablePath)

    return


def initT(key, table, runTablePath):

    prj=qaOpts.getOpt('PROJECT_AS')
    qaHome = qaOpts.getOpt('QA_HOME')
    pwd= os.getcwd()

    pDir=[]

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

    if key.find('check_list') > -1:
        # concatenate existing files
        toCat=[]
        for ix in range(len(pDir)):
            t = os.path.join(qaHome, pDir[ix], table)
            if os.path.isfile(t):
                toCat.append(t)

        # is any of the files newer than the destination?
        dest = os.path.join(runTablePath, table)
        dest_modTime = qa_util.fGetModTime(dest)
        for f in toCat:
            if qa_util.fGetModTime(f) > dest_modTime:
                qa_util.cat(toCat, dest)
                break
    else:
        # just copy the file with highest precedence; ther should only
        # be a single one for each kind of table
        for ix in range(len(pDir)):
            src = os.path.join(qaHome, pDir[ix], table)
            if os.path.isfile(src):
                if not os.path.isdir(runTablePath):
                    qa_util.mkdirP(runTablePath)

                dest= os.path.join(runTablePath, table)
                if qa_util.fGetModTime(src) > qa_util.fGetModTime(dest):
                    shutil.copyfile(src,dest)
                    break

    return


def runExample():
    if qaOpts.isOpt('EXAMPLE_PATH'):
        currdir = os.path.join( qaOpts.getOpt('EXAMPLE_PATH'), 'example')
        if currdir[0] != '/':
            currdir = os.path.join(os.getcwd(), currdir)
    else:
        currdir=os.path.join(QA_SRC, 'example')

    if not qa_util.mkdirP(currdir):
        print 'could not mkdir ' + dir + ', please use option --example=path'
        sys.exit(1)

    os.chdir(currdir)
    qa_util.rmR( 'results', 'config.txt', 'data', 'tables', 'qa-test.task' )

    print 'make examples in ' + currdir
    print 'make qa_test.task'

    taskFile = os.path.join(QA_SRC, 'example', 'templates', 'qa-test.task')
    shutil.copy( taskFile, currdir)
    taskFile = 'qa-test.task'

    # replace templates
    find=[]
    repl=[]

    find.append('PROJECT_DATA=data')
    repl.append('PROJECT_DATA=' + os.path.join(currdir, 'data') )

    find.append('QA_RESULTS=results')
    repl.append('QA_RESULTS=' + os.path.join(currdir, 'results') )

    qa_util.fStrReplace(taskFile, find, repl)

    # data
    print 'make data'

    subprocess.call(["tar", "--bzip2", "-xf", \
        os.path.join(QA_SRC,os.path.join('example', 'templates', 'data.tbz') ) ])

    txtFs=[]
    for rs, ds, fs in os.walk('data'):
        for f in fs:
            if f.find('.txt') > -1:
                txtFs.append(os.path.join(rs,f))

        if qa_util.which("ncgen"):
            for f in txtFs:
                subprocess.call(["ncgen", "-k", "3", "-o", \
                    qa_util.stripLastItem(f,sep='.')+'.nc', f])
                qa_util.rmR(f)
        else:
            print "building data in example requires the ncgen utility"

    print 'run'
    print os.path.join(QA_SRC, 'scripts', 'qa-dkrz') +\
                       " -m --work=" + currdir + '-f qa-test.task'

    subprocess.call([os.path.join(QA_SRC, 'scripts', 'qa-dkrz'), \
                    '--work=' + currdir, "-f", "qa-test.task"])

    return


# -------- main --------
# read (may-be convert) the cfg files
cfgFile=qaOpts.getOpt('CFG_FILE')
qa_util.cfgParser(rawCfgPars, cfgFile, init=True)



if "QA_EXAMPLE" in qaOpts.dOpts:
    runExample()
    sys.exit(0)

init()


final()

if __name__ == '__main__':
    if 'SHOW_CONF' in qaOpts.dOpts:
        mKeys=qaOpts.dOpts.keys()
        mKeys.sort()

        for key in mKeys:
            print '{}={}'.format(key, qaOpts.dOpts[key])
