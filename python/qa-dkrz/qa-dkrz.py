
'''
Created on 17.03.2016

@author: hdh
'''

import sys
import os
import glob
import re
import shutil
import subprocess
import copy

import qa_util
import qa_init
import qa_version

from Queue import Queue
from threading import Thread

from qa_config import QaConfig
#from qa_config import CfgFile
from qa_log import Log
from qa_util import GetPaths
from qa_launcher import QaLauncher
from qa_exec import QaExec
from qa_summary import LogSummary

# struct like classes containing variables
class GlobalVariables(object):
    # valid across the total run
    pass
class ThreadVariables(object):
    # determined at every get_next_variable() call
    pass


def clear(qa_var_path, fBase, logfile):
    if qaConf.isOpt('CLEAR_LOGFILE'):
        try:
            g_vars.clear_fBase
        except:
            g_vars.clear_fBase = []

        # mark for clearance of logfiles if enabled,
        # processing eventually in final().
        ix = g_vars.log_fnames.index(t_vars.log_fname)
        if ix == len( g_vars.clear_fBase):
            g_vars.clear_fBase.append([])

        g_vars.clear_fBase[ix].append(fBase)

    # symbolic link algorithm is skipped

    # clearing section for regular files
    fs=glob.glob(os.path.join(qa_var_path, '*_' + fBase + '*' ) )

    if os.path.exists( os.path.join(qa_var_path, 'core')):
        fs.append('core')

    # remove selected
    for f in fs:
        try:
            os.remove(f)
        except OSError:
            pass

    '''
    # delete table entries
    try:
        entry_beg = subprocess.check_output('grep',
                        '-n',
                        '[[:space:]]*file:[[:space:]]*' + fBase,
                        logfile)

    except:
        pass
    else:
        for i in range(len(entry_beg)):
            line_num = entry_beg[i] - 1

            try:
                subprocess.call( 'sed', '-i', str(line_num) + ',/status:/ d',
                                 logfile)
            except:
                pass
    '''

    return


def clearInq(qa_var_path, fBase, logfile):
#    isFollowLink = False
    v_clear = qaConf.getOpt('CLEAR')

    if repr(type(v_clear)).find('bool') > -1:
        return clear(qa_var_path, fBase, logfile)

    isClear=False

    for f in v_clear.split(','):
        if f == 't':
            break  # unconditional

        '''
        if f == 'follow_links':
            if not getOpt.isOpt('DEREFERENCE_SYM_LINKS'):

                isFollowLink = True
            isClear = True
        el'''

        if f == 'only':
            isClear = True
        elif f[0:4] == 'lock':
            # locked  files
            if len( glob.glob(os.path.join(qa_var_path, 'qa_lock_' + fBase +'*') ) ):
                isClear = True
        elif f[0:4] == 'note':
            if len( glob.glob(os.path.join(qa_var_path, 'qa_note_' + fBase +'*') ) ):
                isClear = True
        elif f[0:4] == 'mark':
            # only pass those that are locked; redo erroneous cases
            if len( glob.glob(os.path.join(qa_var_path, fBase + '.clear') )):
                isClear = True
        elif qa_util.rstrip(f, sep='=') == 'level':
            # clear specified level
            f=qa_util.lstrip(f, sep='=') + '-'
        elif f.find('=') > -1 and qa_util.rstrip(f, '=') == 'tag':
            # e.g. 'L1-${tag}: where tag=CF_12 would match CF_12, CF_12x etc.
            f='[\w]*-*' + qa_util.lstrip(f, sep='=') + '.*: '

            fls = glob.glob(os.path.join(qa_var_path, 'qa_note_' + fBase +'*') )
            fls.extend(glob.glob(os.path.join(qa_var_path, 'qa_lock_' + fBase +'*') ))

            for fl in fls:
                try:
                    subprocess.check_call('grep', '-q', f, fl)
                except:
                    pass
                else:
                    isClear = True
                    break
        else:
            # CLEAR=var=name
            pos = f.find('var=')
            if pos > -1:
                f = f[pos+4:]

            # else:
                # CLEAR=varName

            regExp = re.match(f, fBase)
            if regExp:
                tmp_ls = glob.glob(os.path.join(qa_var_path, '*_' + fBase + '*'))
                if len(tmp_ls):
                    isClear = True

        if isClear:
            # now do the clearance
            return clear(qa_var_path, fBase, logfile)

    return False


def final():

    # only the summary of previous runs
    if not ( qaConf.isOpt('NO_SUMMARY') or qaConf.isOpt('SHOW') ):
        # remove duplicates
        for log_fname in g_vars.log_fnames:
            tmp_log = os.path.join(g_vars.check_logs_path,
                                'tmp_' + log_fname + '.log')
            dest_log = os.path.join(g_vars.check_logs_path,
                                    log_fname + '.log')

            if not os.path.isfile(tmp_log):
                continue  # nothing new

            if os.path.isfile(dest_log):
                if qaConf.isOpt('CLEAR_LOGFILE'):
                    ix = g_vars.log_fnames.index(log_fname)
                    fBase = g_vars.clear_fBase[ix]

                    clrdName='cleared_' + log_fname + '.log'
                    clrdFile=os.path.join(g_vars.check_logs_path, clrdName)

                    with open(clrdFile, 'w') as clrd_fd:
                        while True:
                            blk = log.get_next_blk(dest_log,
                                                skip_fBase=fBase,
                                                skip_prmbl=False)

                            for b in blk:
                                clrd_fd.write(b)
                            else:
                                break

                    if clrd_fd.errors == None:
                        os.rename(clrdFile, dest_log)

                # append recent results to a logfile
                qa_util.cat(tmp_log, dest_log, append=True)
                os.remove(tmp_log)

            else:
                # first time that a check was done for this log-file
                os.rename(tmp_log, dest_log)

    # special: if no project but a single file, then display log-file
    if not qaConf.isOpt("PROJECT_DATA"):
        fname = os.path.join(qaConf.dOpts["QA_RESULTS"], 'check_logs', t_vars.log_fname+'.log')

        with open(fname, 'r') as f:
            print f.read()

        shutil.rmtree(qaConf.dOpts["QA_RESULTS"])

        return

    summary()

    qaConf.cfg.write_file()

    return


def get_all_logfiles():
    f_logs=[]
    if qaConf.isOpt('SHOW_EXP'):
        isShowExp=True
        lfn_var={}

    while True:
        try:
            # fBase: list of filename bases corresponding to variables;
            #          usually a single one. F
            # fNames: corresponding sub-temporal files
            data_path, fBase, fNames = getPaths.next()

        except StopIteration:
            break
        else:
            sub_path = data_path[g_vars.prj_dp_len+1:]
            f_log = qa_util.get_experiment_name(g_vars, qaConf, fB=fBase,
                                            sp=sub_path)
            if isShowExp:
                if f_log in lfn_var.keys():
                    if not sub_path in lfn_var[f_log]:
                        lfn_var[f_log].append(sub_path)
                else:
                    lfn_var[f_log] = [ sub_path ]

            f_logs.append( f_log )

    if isShowExp:
        for key in lfn_var.keys():
            print '\nLogfile-name: ' + key

            for p in lfn_var[key]:
                print '   Variable: ' + p

        sys.exit(0)

    return f_logs


def get_next_variable(data_path, fBase, fNames):

    t_vars.sub_path = data_path[g_vars.prj_dp_len+1:]
    t_vars.var_path = os.path.join(g_vars.res_dir_path, 'data',
                                    t_vars.sub_path )

    # any QA result file of a previous check?
    t_vars.qaFN = 'qa_' + fBase + '.nc'
    qaNc = os.path.join(t_vars.var_path, t_vars.qaFN)

    # experiment_name
    t_vars.log_fname = \
        qa_util.get_experiment_name(g_vars, qaConf, fB=fBase,
                                    sp=t_vars.sub_path)
    t_vars.pt_name = \
        qa_util.get_project_table_name(g_vars, qaConf, fB=fBase,
                                       sp=t_vars.sub_path)

    # Any locks? Any clearance of previous results?
    if testLock(t_vars, fBase):
        return []

    syncCall = g_vars.syncFilePrg + ' --only-marked'

    if os.path.exists(qaNc):
        syncCall += ' -p ' + qaNc

    if qaConf.isOpt('TIME_LIMIT'):
        syncCall += ' -l ' + qaConf.getOpt('TIME_LIMIT')

    syncCall += ' -P ' + data_path
    for f in fNames:
        syncCall += ' ' + f

    try:
        next_file_str = subprocess.check_output(syncCall, shell=True)

    except subprocess.CalledProcessError as e:
        status = e.returncode

        if status == 1:
            return []

        info = e.output.splitlines()
        isLog=False

        if status == 3:
            g_vars.anyProgress = True

            entry_id = log.append(t_vars.log_fname, f=fBase, d_path=data_path,
                       r_path=g_vars.res_dir_path, sub_path=t_vars.sub_path,
                       caption='unspecific error in sub-temporal file sequence',
                       impact='L2', tag='M5', info=info,
                       conclusion='DRS(F): FAIL', status=status)
            isLog = True

        elif status == 4:
            # a fixed variable?
            if len(qaNc):
                # previously processed
                return []

        elif status > 10:
            g_vars.anyProgress = True

            entry_id = log.append( t_vars.log_fname, f=fBase, d_path=data_path,
                        r_path=g_vars.res_dir_path, sub_path=t_vars.sub_path,
                        caption='ambiguous sub-temporal file sequence',
                        impact='L2', tag='M6', info=info,
                        conclusion='DRS(F): FAIL', status=status)
            isLog = True

        if isLog:
            log.write_entry(entry_id,
                            g_vars.check_logs_path,
                            t_vars.log_fname)

            return []

    lst = next_file_str.splitlines()

    return lst


def get_version(qaConf):

    com_line_opts={}

    # this is mandatory
    com_line_opts["SECTION"] = qaConf.getOpt("QA_SRC")

    if qaConf.isOpt('SHOW_VERSION'):
        com_line_opts["SHOW_VERSION"]=True

    '''
    if not qaConf.dOpts['SHOW_VERSION'] == 't':
        com_line_opts["VERBOSE"] = True

    if qaConf.isOpt('PROJECT'):
        com_line_opts["PROJECT"] = qaConf.getOpt("PROJECT")

    if qaConf.isOpt('CONDA_PATH'):
        com_line_opts["IS_CONDA"] = True
    '''

    rev = qa_version.get_version( opts=qaConf.dOpts,
                                  com_line_opts=com_line_opts)

    qaConf.dOpts["QA_REVISION"] = rev

    if qaConf.isOpt('SHOW_VERSION'):
        print rev
        sys.exit(0)

    return


def run():

    if qaConf.isOpt("SHOW_EXP"):
        f_log = get_all_logfiles()
        sys.exit(0)

    if qaConf.isOpt('SHOW') or qaConf.isOpt('NEXT'):
        g_vars.thread_num = 1

    # the queue is two items longer than the number of threads
    queue = Queue(maxsize=g_vars.thread_num+2)

    launch_list = []

    if g_vars.thread_num < 2:
        # a single thread
        qaExec = QaExec(log, qaConf, g_vars)
        launch_list.append( qaExec )
    else:
        for i in range(g_vars.thread_num):
            launch_list.append( QaLauncher(log, qaConf, g_vars) )

        for i in range(g_vars.thread_num):
            t = Thread(target=launch_list[i].start, args=(queue,))
            t.daemon = True
            t.start()

    isNoPath=True

    is_next_var=False
    if qaConf.isOpt('NEXT_VAR'):
        is_next_var=True
        next_var=qaConf.getOpt('NEXT_VAR')
        count_next_var=0

    while True:

        if is_next_var:
            if count_next_var == next_var:
                break

            count_next_var += 1

        try:
            # fBase: list of filename bases corresponding to variables;
            #          usually a single one. F
            # fNames: corresponding sub-temporal files
            data_path, fBase, fNames = getPaths.next()

        except StopIteration:
            if isNoPath:
                print "PROJECT_DATA: " + qaConf.getOpt("PROJECT_DATA") + " not found."

            queue.put( ('---EOQ---', '', t_vars), block=True)
            break
        else:
            isNoPath=False

            # return a list of files which have not processed, yet.
            # Thus, the list could be empty
            fL = get_next_variable(data_path, fBase, fNames)
            #fL[0]=fL[0][:-2]

            if len(fL) == 0:
                continue

            t_vars.fBase = fBase

            #queue.put( (data_path, fL, t_vars), block=True)
            queue.put( (data_path, fL, copy.deepcopy(t_vars)),
                        block=True)

        if g_vars.thread_num < 2:
            # a single thread
            if not launch_list[0].start(queue,):
                break

    if g_vars.thread_num > 1:
        queue.join()

    if g_vars.thread_num < 2:
        launch_list[0].printStatusLine()
    else:
        launch_list[0].qa_exec.printStatusLine()

    return


def runExample():
    if qaConf.isOpt('EXAMPLE_PATH'):
        currdir = os.path.join( qaConf.getOpt('EXAMPLE_PATH'), 'example')
        if currdir[0] != '/':
            currdir = os.path.join(os.getcwd(), currdir)
    else:
        currdir=os.path.join(QA_SRC, 'example')

    if not qa_util.mkdirP(currdir):
        print 'could not mkdir ' + dir + ', please use option --example=path'
        sys.exit(1)

    os.chdir(currdir)
    qa_util.rm_r( 'results', 'config.txt', 'data', 'tables', 'qa-test.task' )

    print 'make examples in ' + currdir
    print 'make qa_test.task'

    taskFile = os.path.join(QA_SRC, 'example', 'templates', 'qa-test.task')
    shutil.copy( taskFile, currdir)
    taskFile = 'qa-test.task'

    # replace templates
    sub=[]
    repl=[]

    sub.append('PROJECT_DATA=data')
    repl.append('PROJECT_DATA=' + os.path.join(currdir, 'data') )

    sub.append('QA_RESULTS=results')
    repl.append('QA_RESULTS=' + os.path.join(currdir, 'results') )

    qa_util.f_str_replace(taskFile, sub, repl)

    # data
    print 'make data'

    subprocess.call(["tar", "--bzip2", "-xf", \
        os.path.join(QA_SRC,os.path.join('example', 'templates', 'data.tbz') ) ])

    txtFs=[]
    for rs, ds, fs in os.walk('data'):
        for f in fs:
            if '.txt' in f:
                txtFs.append(os.path.join(rs,f))

        if qa_util.which("ncgen"):
            for f in txtFs:
                nc_f = f[:len(f)-2] + 'nc'

                subprocess.call(["ncgen", "-k", "3", "-o", nc_f, f])

                qa_util.rm_r(f)
        else:
            print "building data in example requires the ncgen utility"

    print 'run'
    print os.path.join(QA_SRC, 'python', 'qa-dkrz') +\
                       " -m --work=" + currdir + '-f qa-test.task'

    subprocess.call([os.path.join(QA_SRC, 'python', 'qa-dkrz'), \
                    '--work=' + currdir, "-f", "qa-test.task"])

    return


def summary():
    # preparation to call a summary object
    if qaConf.isOpt('NO_SUMMARY'):
       return

    if qaConf.isOpt('ONLY_SUMMARY'):
        # build only the summary of previously written log-files.
        f_log = qaConf.getOpt('ONLY_SUMMARY')
        if repr(type(f_log)).find('str') > -1:
            f_log = [f_log]

        if repr(type(f_log)).find('list') == -1:
            # all the log-files corresponding to the current selection
            f_log = get_all_logfiles()
            f_log.append(g_vars.check_logs_path)

    f_log = g_vars.log_fnames
    f_log.append(g_vars.check_logs_path)

    # summary object
    log_sum = LogSummary()
    f_log = log_sum.prelude(f_log)
    for i in range( len(f_log) ):
        log_sum.run(f_log[i])

    if qaConf.isOpt('ONLY_SUMMARY'):
        sys.exit(0)

    return


def testLock(t_vars, fBase):
    if qaConf.isOpt('CLEAR'):
        # note that the logfile is temporary, finished in final()
        logfile = os.path.join(g_vars.check_logs_path,
                                    'tmp_' + t_vars.log_fname + '.log')

        if clearInq(t_vars.var_path, fBase, logfile):
            return True

    # any qa_lock file?
    f_lock = os.path.join(t_vars.var_path, 'qa_lock_' + fBase + '.txt')
    if os.path.exists(f_lock):
        return True

    return False


if __name__ == '__main__':
    (isCONDA, QA_SRC) = qa_util.get_QA_SRC(sys.argv[0])

    # for options on the command-line as well as in configuration files
    qaConf=QaConfig(QA_SRC)

    if not qaConf.isOpt("QA_TABLES"):
        qa_init.run_inq_tables()

    g_vars = GlobalVariables()
    t_vars = ThreadVariables()

    g_vars.qa_src = QA_SRC
    g_vars.isConda = isCONDA
    g_vars.pid = str(os.getpid())

    log = Log(qaConf.dOpts)

    if isCONDA:
        qaConf.addOpt('CONDA', True)

    # obj for getting and iteration next path
    getPaths = GetPaths(qaConf)

    if 'QA_EXAMPLE' in qaConf.dOpts:
        runExample()
        sys.exit(0)

    qa_init.run(log, g_vars, qaConf)

    get_version(qaConf)

    # the checks
    if not qaConf.getOpt('ONLY_SUMMARY'):
        run()

    final()

    if 'SHOW_CONF' in qaConf.dOpts:
        for opt in qa_util.get_sorted_options(qaConf.dOpts):
            # opt: key=value
            print opt
