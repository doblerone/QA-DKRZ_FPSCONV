'''
Created on 21.03.2016

@author: hdh
'''

import os
import glob
import subprocess
import Queue

import qa_util

class QaExec(object):
    '''
    classdocs
    '''

    def __init__(self, log, qaOpts, g_vars):
        # instances for logging results, provided options and a 'python struct'
        self.log = log
        self.qaOpts = qaOpts
        self.g_vars = g_vars

        self.is_show_call=False
        if qaOpts.isOpt('SHOW_CALL'):
            self.is_show_call=True

        self.is_next=False
        if qaOpts.isOpt('NEXT'):
            self.is_next=True
            self.next=qaOpts.getOpt('NEXT')

        self.capt     = 'CAPT'
        self.capt_len = len(self.capt)
        self.beg      = '-BEG'
        self.end      = '-END'
        self.len_beg  = len(self.beg)
        self.len_end  = len(self.end)


    def getParamStr(self, t_vars):
        par =  ' -p ' + t_vars.data_path
        par += ' -f ' + t_vars.fName
        par += ' -t ' + self.g_vars.table_path

        par += self.get_X_par(t_vars)
        par += self.get_CF_par(t_vars)
        par += self.get_IN_par(t_vars)
        par += self.get_QA_par(t_vars)
        if self.qaOpts.isOpt('TIME_LIMIT'):
            par += self.get_TC_par(t_vars)
        if self.qaOpts.isOpt('FREQ_DIST'):
            par += self.get_FD_par(t_vars)

        return par


    def get_CF_par(self, t_vars, inst_ix=0):
        qaOpts = self.qaOpts

        # construct directives for the annotation handling
        par = self.get_X_par(t_vars, inst_ix=1)

        # combine objects
        par += ' CF::0:IN::0:X::1'

        if qaOpts.isOpt('CF'):
            par += ':cF=' + qaOpts.getOpt('CF')

        if qaOpts.isOpt('CF_AREA_TYPES'):
            par += ':cFAT=' + qaOpts.getOpt('CF_AREA_TYPES')

        if qaOpts.isOpt('CF_STANDARD_NAMES'):
            par += ':cFSN=' + qaOpts.isOpt('CF_STANDARD_NAMES')

        if qaOpts.isOpt('CF_STD_REGION_NAMES'):
            par += ':cFSRN=' + qaOpts.getOpt('CF_STD_REGION_NAMES')

        if qaOpts.isOpt('CF_FOLLOW_RECOMMENDATIONS'):
            par += ':fR'

        return par


    def get_IN_par(self, t_vars, inst_ix=0):
        qaOpts = self.qaOpts

        par = ' IN::0:X::0'

        if qaOpts.isOpt('ARITHMETIC_MEAN'):
            par += ':aM'

        if qaOpts.isOpt('DISABLE_INF_NAN'):
            par += ':dIN'

        if qaOpts.isOpt('EXCLUDE_VARIABLE'):
            s=','
            par += ':eV=' + s.join(qaOpts.getOpts('EXCLUDE_VARIABLE'))

        return par


    def get_QA_par(self, t_vars, inst_ix=0):
        qaOpts = self.qaOpts

        par = ' QA::0:IN::0:CF::0:X::0'
        par += ':f=' + t_vars.qaFN
        par += ':fS=' + t_vars.seq_pos

        if qaOpts.isOpt('APPLY_MAXIMUM_DATE_RANGE', inq_bool=True):
            par += ':aMDR'
        elif qaOpts.isOpt('APPLY_MAXIMUM_DATE_RANGE'):
            par += ':aMDR=' + qaOpts.getOpt('APPLY_MAXIMUM_DATE_RANGE')

        if qaOpts.isOpt('CHECK_MODE'):
            s=','
            par += ':cM=' + s.join(qaOpts.getOpts('CHECK_MODE'))

        if qaOpts.isOpt('DATA_IN_PRODUCTION'):
            par += ':dIP'

        if qaOpts.isOpt('DISABLE_CONSISTENCY_CHECK'):
            par += ':dCC'

        if qaOpts.isOpt('DISABLE_TIME_BOUNDS_CHECK'):
            par += ':dTB'

        if qaOpts.isOpt('EXCLUDE_ATTRIBUTE'):
            s=','
            par += ':eA=' + s.join(qaOpts.getOpts('EXCLUDE_ATTRIBUTE'))

        if qaOpts.isOpt('FREQUENCY'):
            par += ':fq=' + qaOpts.getOpt('FREQUENCY')

        if qaOpts.isOpt('FREQUENCY_POSITION'):
            par += ':fqp=' + qaOpts.getOpt('FREQUENCY_POSITION')

        if qaOpts.isOpt('IGNORE_REF_DATE_ACROSS_EXP'):
            par += ':iRDAE=' + qaOpts.getOpt('IGNORE_REF_DATE_ACROSS_EXP')

        if qaOpts.isOpt('IGNORE_REFERENCE_DATE'):
            par += ':iRD'

        if qaOpts.isOpt('NEXT_RECORDS'):
            par += ':nextRecords=' + qaOpts.getOpt('NEXT_RECORDS')

        if qaOpts.isOpt('NON_REGULAR_TIME_STEP'):
            par += ':nRTS=' + qaOpts.getOpt('NON_REGULAR_TIME_STEP')

        if qaOpts.isOpt('OUTLIER_TEST'):
            s=','
            par += ':oT=' + s.join(qaOpts.getOpt('OUTLIER_TEST'))

        if qaOpts.isOpt('PARENT_EXP_ID'):
            par += ':pEI=' + qaOpts.getOpt('PARENT_EXP_ID')

        if qaOpts.isOpt('PARENT_EXP_RIP'):
            par += ':pER=' + qaOpts.getOpt('PARENT_EXP_RIP')

        if t_vars.post_proc:
            par += ':pP'

        if qaOpts.isOpt('PRINT_GREP_MARK'):
            par += ':pGM'

        # PROJECT_TABLE
        par += ':tPr=' + t_vars.pt_name

        if qaOpts.isOpt('QA_NCFILE_FLAGS'):
            par += ':qNF=' + qaOpts.getOpt('QA_NCFILE_FLAGS')

        if qaOpts.isOpt('REPLICATED_RECORD'):
            s=','
            par += ':rR=' + s.join(qaOpts.getOpt('REPLICATED_RECORD'))

        if qaOpts.isOpt('TABLE_DRS_CV'):
            par += ':tCV=' + qaOpts.getOpt('TABLE_DRS_CV')

        if qaOpts.isOpt('TABLE_GCM_NAME'):  # CORDEX
            par += ':tGCM=' + qaOpts.getOpt('TABLE_GCM_NAME')

        if qaOpts.isOpt('TABLE_RCM_NAME'):  # CORDEX
            par += ':tRCM=' + qaOpts.getOpt('TABLE_RCM_NAME')

        if qaOpts.isOpt('TABLE_ATT_REQ'):  # CMIP5
            par += ':tAR=' + qaOpts.getOpt('TABLE_ATT_REQ')

        if qaOpts.isOpt('TABLE_EXPERIMENT'):
            par += ':tX=' + qaOpts.getOpt('TABLE_EXPERIMENT')

        if qaOpts.isOpt('TABLE_FORCING_DESCRIPT'):
            par += ':tFD=' + qaOpts.getOpt('TABLE_FORCING_DESCRIPT')

        if qaOpts.isOpt('TABLE_TIME_SCHEDULE'):
            par += ':tTR=' + qaOpts.getOpt('TABLE_TIME_SCHEDULE')

        if qaOpts.isOpt('TABLE_VAR_REQ'):
            par += ':tVR=' + qaOpts.getOpt('TABLE_VAR_REQ')

        if qaOpts.isOpt('TABLE_RELAXED_DIM_CONSTRAINT'):
            par += ':tRDC=' + qaOpts.getOpt('TABLE_RELAXED_DIM_CONSTRAINT')

        if qaOpts.isOpt('TABLE_RELAXED_DIM_CONSTRAINT'):
            par += ':uS'

        return par


    def get_X_par(self, t_vars, inst_ix=0):
        par = ' X::' + str(inst_ix)

        if inst_ix == 0:
            note             = self.qaOpts.getOpt('NOTE')
            note_always      = self.qaOpts.getOpt('NOTE_ALWAYS')
            note_level_limit = self.qaOpts.getOpt('NOTE_LEVEL_LIMIT')
            checklist        = self.qaOpts.getOpt('QA_CHECK_LIST')
        else:
            note             = self.qaOpts.getOpt('CF_NOTE')
            note_always      = self.qaOpts.getOpt('CF_NOTE_ALWAYS')
            note_level_limit = self.qaOpts.getOpt('CF_NOTE_LEVEL_LIMIT')
            checklist        = self.qaOpts.getOpt('CF_CHECK_LIST')

            # suppress printing of check results
            par += ':nCR'

        if len(note):
            par += ':note=' + note
        if len(note_always):
            par += ':nA=' + note_always
        if len(note_level_limit):
            par += ':nLL=' + note_level_limit
        if len(checklist):
            par += ':cL=' + checklist

        return par


    def get_TC_par(self, t_vars, inst_ix=0):
        par = ' TC::' + str(inst_ix)
        par += ":e=" + self.qaOpts.getOpt('TIME_LIMIT')

        return par


    def get_FD_par(self, t_vars, inst_ix=0):
        qaOpts = self.qaOpts

        par = ' FD::' + str(inst_ix)
        par += 'useAreaWeight'

        if qaOpts.isOpt('FD_PLAIN'):
            par += ':printPlain'

        if qaOpts.isOpt('FD_BARS'):
            par += ':printBars'

        if qaOpts.isOpt(''):
            par += ':=' + qaOpts.getOpt('')

        # A fd-build-file of the same variable at destination has priority.
        # Second choice is a fd-build-file or fd-prop-file in a specified
        # path. Third choice is an explict statement of properties.
        # Default: automatic determination of fd properties.

        # Directive for saving the output to a build-file
        # Note: the effective name will have a time interval appended.
        fName = 'fd_' + t_vars.fBase
        par += ':s=' + fName + '.build'

        if qaOpts.isOpt('FD_TIME_PART'):
            par += ':part=' + qaOpts.getOpt('FD_TIME_PART')

        # Properties of the freq. dist. Note: current dir is destination.
        files = glob.glob(fName + '*.build')
        if len(files):
            # Rule 1: rebuild, i.e continue a FD
            par += ':r=' + files[0]  # read properties
        elif qaOpts.isOpt('FD_PROPERTY_PATH'):
            files = glob.glob(os.path.join( qaOpts.getOpt('FD_PROPERTY_PATH'),
                                            fName + '*.build'))

            if len(files):
                # Rule 2a
                par += ':r=' + files[0]
            else:
                files = glob.glob(os.path.join( qaOpts.getOpt('FD_PROPERTY_PATH'),
                                                fName + '*.prop'))
                if len(files):
                    # Rule 2b  read properties
                    par += ':rp=' + files[0]
        elif qaOpts.isOpt('FD_EXPLICIT_PROPS'):
            props = qaOpts.getOpt('FD_EXPLICIT_PROPS').split('/')
            par += ':w=' + props[0] + '@i=' + props[1]

        return par


    def parse_FLAG(self, key, issue, log_entry):
        # replace: " --> '
        while True:
            p0 = issue.find('"')
            if p0 > -1:
                issue = issue[:1] + "'" + issue[2:]
            else:
                break

        add={}
        remainder = self.parse_output(issue, log_entry, dct=add)

        if not 'event' in log_entry:
            # only once
            log_entry['is_event'] = True
            log_entry['event'] = []

        event = {}
        p0 = remainder.find(':')
        event['impact'] = issue[:p0]

        p0 = event['impact'].find('-')
        event['tag'] = event['impact'][p0+1:]

        p0 = event['impact'].rfind('-')
        event['impact'] = event['impact'][:p0]

        for k in add.keys():
            event[k] = add[k]

        if len(event):
            log_entry['event'].append(event)

        return


    def parse_PERIOD(self, key, issue, log_entry):
        lst = issue.split()
        log_entry[key] = 'period:'

        if len(lst) == 3:
            log_entry[key+self.beg] = 'begin: ' + lst[0]
            log_entry[key+self.end] = 'end: ' + lst[2]
        elif len(lst) == 2:
            if lst[0] == '-':
                log_entry[key+self.beg] = 'begin: miss'
                log_entry[key+self.end] = 'end: ' + lst[1]
            else:
                log_entry[key+self.beg] = 'begin: ' + lst[1]
                log_entry[key+self.end] = 'end: miss'
        else:
            log_entry[key+self.beg] = 'begin: miss'
            log_entry[key+self.end] = 'end: miss'

        return


    def parse_output(self, check_output, log_entry, dct={}):
        # parse the output from  the C++ executable

        # split according to key-words
        keys = [ 'FLAG', 'CAPT', 'TXT', 'CHECK', 'STATUS', 'CREATE', 'TRACK',
                 'PERIOD', 'CPU', 'REAL', 'EMAIL', 'SUBJECT']

        # for multiple, embedded occurrences of keys
        ixs = range(len(keys))

        while True:
            not_found=True

            # find always the key-word that comes first
            p_min=len(check_output)
            for ix in ixs:
                p0 = check_output.find(keys[ix]+self.beg)

                if p0 > -1 and p0 < p_min:
                    p_min = p0
                    p_ix = ix
                    not_found = False
                    if p0 == 0:
						break

            if not_found:
                break

            key = keys[p_ix]

            front='' # frontal residuum
            back=''  # residuum of the back-side

            #p0 = check_output.find(key+self.beg)
            p0 = p_min
            p1 = check_output.find(key+self.end)

            if p1 > -1:
                front = check_output[0:p0]
                back  = check_output[p1+len(key)+self.len_end:]
                issue = check_output[p0+len(key)+self.len_beg:p1]
                check_output = front + back

                if key == 'CHECK':
                    log_entry['conclusion'] = issue

                elif key == 'PERIOD':
                    self.parse_PERIOD(key, issue, log_entry)

                elif key == 'FLAG':
                    self.parse_FLAG(key, issue, log_entry)

                elif key == 'CAPT':
                    # reached out of parse_FLAG
                    dct['caption'] = issue.strip()

                elif key == 'TXT':
                    # reached out of parse_FLAG

                    # additional info between TXT, multiple lines
                    # internally separated by ';'
                    issue = issue.strip(' ;')

                    if issue.find(';') > -1:
                        dct['info'] = issue.split(';')
                    else:
                        dct['info'] = [ issue ]

                elif key == 'STATUS':
                    log_entry['status'] = issue
                else:
                    log_entry[key] = issue

        return check_output


    def run(self, t_vars):

        qa_lock = os.path.join(t_vars.var_path,
                               'qa_lock_' + t_vars.fBase + '.txt')
        if os.path.isfile(qa_lock):
            return False

        self.nc_file = os.path.join(t_vars.data_path, t_vars.fName)

        if self.qaOpts.isOpt('DRY_RUN'):
            print self.nc_file
            return True

        param = self.getParamStr(t_vars)

        if self.is_show_call:
            print param

        log_entry={}

        qa_util.mkdirP(t_vars.var_path)

        cpp_run = self.g_vars.checkPrg + param
        set_qa_lock = False

        try:
            check_output = subprocess.check_output(cpp_run,
                                                shell=True,
                                                cwd=t_vars.var_path)
        except subprocess.CalledProcessError as e:
            istatus = e.returncode
            set_qa_lock = True

            if istatus == 63:
                return True

            check_output = e.output

            # atomic variable gets locked
            if istatus > 1:
                set_qa_lock = True

            if istatus == 4:
                pass
                # EMERGENCY STOP
                # isSignalTerm = t
                # issueMail = t

            elif istatus > 4:
                # uncontroled system exception, e.g. segmentation fault
                log_entry['conclusion'] = 'segmentation fault'
                log_entry['is_event'] = True
                log_entry['event'] = []
                event = {}
                event['caption']   = 'run-time error: segmentation fault'
                event['impact'] = 'L2'
                event['tag']    ='S_1'
                log_entry['event'].append(event)

        else:
            istatus = 0

        entry_id=''

        # prepare the logfile entry
        if istatus < 5:
            self.parse_output(check_output, log_entry)

            entry_id = self.log.append( entry_id,
                                        f           = t_vars.fName,
                                        d_path      = t_vars.data_path,
                                        r_path      = t_vars.var_path,
                                        conclusion  = log_entry['conclusion'],
                                        set_qa_lock = set_qa_lock)

        if 'is_event' in log_entry.keys():
            entry_id = self.log.append(entry_id, is_events=True)

            for eve in log_entry['event']:
                entry_id = self.log.append( entry_id,
                                            caption  = eve['caption'],
                                    impact   = eve['impact'],
                                    tag      = eve['tag'])

            if 'info' in eve.keys():
                entry_id = self.log.append(entry_id, info=eve['info'])

        entry_id = self.log.append(entry_id, status=istatus)

        self.log.write_entry(entry_id,
                       self.g_vars.check_logs_path,
                       t_vars.log_fname)

        if istatus > 1:
            proceed = False
        else:
            proceed = True

        return proceed  # true: proceed with next sub-temporal file


    def start(self, queue):
        # only called for non-thread operation

        while True:
            try:
                (data_path, fNames, t_vars) = queue.get(False)
            except Queue.Empty:
                break
            else:
                if data_path == '---EOQ---':
                    break

                lenFN_1=len(fNames)-1

                for ix in range(lenFN_1+1):
                    is_post_proc = False

                    if self.is_next:
                        self.next -= 1
                        if self.next == -1:
                            break

                    if ix == 0:
                        if ix == lenFN_1:
                            seq_pos = 'x'  # there is only a single file
                            is_post_proc = True
                        else:
                            seq_pos = 'f'
                    elif ix == lenFN_1:
                        seq_pos = 'l'
                        is_post_proc = True
                    else:
                        seq_pos = 's'

                    t_vars.data_path = data_path
                    t_vars.fName = fNames[ix]
                    t_vars.seq_pos = seq_pos
                    t_vars.post_proc = is_post_proc

                    if not self.run(t_vars):
                        break

                queue.task_done()

                # for the previous range == 1
                if self.is_next and self.next == 0:
                    return False

        return True
