'''
Created on 21.03.2016

@author: hdh
'''

import sys
import os
import shutil
import socket

import argparse
#import argcomplete
#import subprocess

import ConfigParser

import qa_util

class QaOptions(object):
    '''
    classdocs
    '''

    def __init__(self, qa_src):
        '''
        Constructor
        '''
        self.qaSrc = qa_src

        # dictionary for the final result

        self.dOpts={}

        self.project=''
        self.lSelect=[]
        self.lLock=[]

        self.project = ''

        # a list of dicts, first for the command-line options,
        # then those from successive QA_CONF files.
        self.ldOpts=[]
        self.cl_ix=-1 # eventually, index of ldOpts for command-line options

        if len(qa_src):
            self.ldOpts.append({})
            self.ldOpts[0]['QA_SRC']=qa_src

        # dictionaries with options( precedence: high --> low)
        self.commandLineOpts( self.create_parser() )

        # the location of tables must be known before config files are analysed.
        self.setHomeDir()

        self.readConfigFiles()
        self.setDefault()

        self.backward_compat()

        self.mergeOptions()

        self.finalSelLoc()

        self.dOpts['CFG_FILE']=os.path.join(os.getenv('HOME'),
                                            '.qa-dkrz',
                                            self.dOpts['CFG_FILE'])
        if 'PROJECT' in self.dOpts.keys():
            if not 'PROJECT_AS' in self.dOpts.keys():
                self.dOpts['PROJECT_AS']=self.project
        elif 'PROJECT_AS' in self.dOpts.keys():
            self.dOpts['PROJECT']=self.getOpt('PROJECT_AS')

        # if any opt is a SHOW_...
        for key in self.dOpts.keys():
            if key[:4] == 'SHOW':
                self.dOpts['SHOW'] = True
                break

        self.ldOpts=[] # free mem
        self.lSelect=[]
        self.lLock=[]


    def addOpt(self, key, val, dct={}):
        if len(dct):
            local_dct = dct
        else:
            local_dct = self.dOpts

        local_dct[key]=val

        return


    def backward_compat(self):
        # rather intricate, but ensures later correct merging of options
        for ldo in self.ldOpts:
            if len(ldo):
                if self.isOpt('EXP_NAME', dct=ldo):
                    self.addOpt('LOG_FNAME', ldo['EXP_NAME'], dct=ldo)
                    del self.dOpts['EXP_NAME']

                if self.isOpt('EXP_FNAME_PATTERN', dct=ldo):
                    self.addOpt('LOG_FNAME_PATTERN', ldo['EXP_FNAME_PATTERN'],
                                dct=ldo)
                    del ldo['EXP_FNAME_INDEX']

                if self.isOpt('EXP_PATH_BASE', dct=ldo):
                    self.addOpt('DRS_PATH_BASE', ldo['EXP_PATH_BASE'], dct=ldo)
                    del ldo['EXP_PATH_BASE']

                if self.isOpt('EXP_PATH_INDEX', dct=ldo):
                    self.addOpt('LOG_PATH_INDEX', ldo['EXP_PATH_INDEX'],
                                dct=ldo)
                    del ldo['EXP_PATH_INDEX']

                if self.isOpt('SHOW_EXPS', dct=ldo):
                    self.addOpt('SHOW_EXP', ldo['SHOW_EXPS'], dct=ldo)
                    del ldo['SHOW_EXPS']

                if self.isOpt('SUMMARY_ONLY', dct=ldo):
                    self.addOpt('ONLY_SUMMARY', ldo['SUMMARY_ONLY'], dct=ldo)
                    del ldo['SUMMARY_ONLY']

                if self.isOpt('SUMMARY', dct=ldo):
                    self.addOpt('ONLY_SUMMARY', ldo['SUMMARY'], dct=ldo)
                    del ldo['SUMMARY']

        return


    def commandLineOpts(self, parser):
        # Some conversions of command-line parameters; just to be back-ward
        # compatible
        lArgv = sys.argv[1:]

        for i in range(len(lArgv)):
            str0=lArgv[i]
            strLen=len(str0)

            if strLen > 2:
                # conversion: remove a '_' after the option
                if str0[0:3] == '-e_' or str0[0:3] == '-E_':
                    str0='-e' + str0[3:]
                elif str0[0:2] == '-E':
                    str0='-e' + str0[2:]

                # convert long-option key-words to lower-case,
                # e.e. --ABC=qWeR --> --abc=qWeR
                if str0[0:2] == '--' :
                    pos = str0.find('=')
                    if not str0[:pos].islower():
                        if pos == -1:
                            str0=str0.lower()
                        else:
                            str0=str0[:pos].lower() + str0[pos:]

            lArgv[i] = str0

        if len(lArgv) == 0: # calling without any parameter raises help
            lArgv.append('-h')

        # let's parse
        args = parser.parse_args(lArgv)

        # post-processing: namespace --> dict
        self.ldOpts.append({})
        self.cl_ix = len(self.ldOpts)-1
        _ld0 = self.ldOpts[self.cl_ix]

        # apply for the list provided by -e_
        if args.eTypeOpt != None:
            for i in range(len(args.eTypeOpt)):
                # convert the key-words to lower-case,
                pos = args.eTypeOpt[i].find('=')
                if pos == -1:
                    key=args.eTypeOpt[i].upper()
                    val=True
                else:
                    key=args.eTypeOpt[i][:pos].upper()
                    val=args.eTypeOpt[i][pos+1:]
                    if val == 't':
                        val = True
                    elif val == 'f':
                        val = False

                # special
                if key == 'NEXT' and val == True:
                    args.NEXT = 1
                    val = 1

                # convert comma-sep val to list
                #if repr(type(val)).find('str') > -1:
                #    val = val.split(',')

                # comma-sep list --> python list, but protect
                # those containing braces
                if repr(type(val)).find('str') > -1:
                    if val.find('{') == -1:
                        try:
                            # --> list of integers
                            val=[ int(x) for x in val.split(',') ]
                            if len(val) == 1:
                                val = val[0]
                        except ValueError:
                            # --> list of strings
                            s=repr(val)
                            if s.find(',') > -1:
                                val=val.split(',')

                            if len(val) == 1:
                                val = val[0]
                        except:
                            pass

                _ld0[key] = val

        # special: long-opts
        if args.PROJECT    != None: _ld0['PROJECT']    = args.PROJECT
        if args.PROJECT_AS != None: _ld0['PROJECT_AS'] = args.PROJECT_AS
        if args.TASK       != None: _ld0['TASK']       = args.TASK
        if args.UPDATE     != None: _ld0['UPDATE']     = args.UPDATE
        if args.QA_HOME    != None: _ld0['QA_HOME']    = args.QA_HOME

        if args.SHOW_NEXT > 0:
            _ld0['NEXT']      = args.SHOW_NEXT
            args.SHOW_CALL = True

        if args.DRYRUN:    _ld0['DRY_RUN']   = True
        if args.NEXT  > 0: _ld0['NEXT']      = args.NEXT
        if args.SHOW_CALL: _ld0['SHOW_CALL'] = args.SHOW_CALL
        if args.SHOW_CONF: _ld0['SHOW_CONF'] = args.SHOW_CONF
        if args.SHOW_EXP:  _ld0['SHOW_EXP']  = args.SHOW_EXP
        if args.SHOW_VERSION: _ld0['SHOW_VERSION'] = args.SHOW_VERSION
        if args.QA_EXAMPLE:
            _ld0['QA_EXAMPLE']  = args.QA_EXAMPLE

        # special: SELECT | LOCK
        if len(args.NC_FILE) == 0:
            if args.CL_L != None:
                s= 'LOCK ' + args.CL_L
                self.lLock.append(self.setSelLock(s.replace(' ','') ) )
            if args.CL_S != None:
                s= 'SELECT ' + args.CL_S
                self.lSelect.append(self.setSelLock(s.replace(' ','') ) )
        else:
            str0='SELECT='
            for i in range(len(args.NC_FILE)):
                if i > 0:
                    str0 += ','
                str0 += args.NC_FILE[i]

            _ld0['SELECT'] = str0

        # collect for passing to install.py
        str0=''
        if args.AUTO_UP    != None:
            str0 += '--auto_up=' + args.AUTO_UP + ','
        if args.AUTO_TABLE_UP    != None:
            str0 += '--auto_table_up=' + args.AUTO_TABLE_UP + ','
        if args.BUILD :
            str0 += '--build' + ','
        if args.LINK != None:
            str0 += '--link=' + args.LINK + ','
        if args.QA_HOME != None:
            str0 += '--qa_home=' + args.QA_HOME + ','
        if args.SET_DEFAULT_PROJECT != None:
            str0 += '--set_default_project=' + args.SET_DEFAULT_PROJECT + ','
        if args.UPDATE != None:
            str0 += '--up=' + args.UPDATE + ','

        if len(str0): # with trailing ',' deleted
            _ld0['install_args']=str0[0:len(str0)-1]

        return


    def create_parser(self):
        parser = argparse.ArgumentParser(
            prog="qa-dkrz",
            description="Meta-data checker QA-DKRZ of climate data NetCDF files."
            )

        parser.add_argument('--dry', '--dry-run', dest='DRYRUN',
            action="store_true",
            help='Show only filenames which are to be processed')

        parser.add_argument( '--example',
            action="store_true", dest='QA_EXAMPLE',
            help="Run the example.")

        parser.add_argument( '-e', '-E',
            action='append', dest='eTypeOpt',
            help="QA configuration: [-e|-E]key[=value].")

        parser.add_argument('-f', '--task', dest='TASK',
            help="QA Configuration file.")

        parser.add_argument('-L', '--lock', dest='CL_L',
            help="Exclude from checking, syntax see LOCK in QA configure file.")

        parser.add_argument('-n', '--next', dest='NEXT',
            type=int, nargs='?', default=0, const=1,
            help="Proceed for a limited number of variables, unlimited by default.")

        parser.add_argument('-P', '--project', dest='PROJECT',
            help= "PROJECT name.")

        parser.add_argument('--project_as', '--project-as', dest = 'PROJECT_AS',
            help= "Use assigned project for processing but PROJECT name is kept.")

        parser.add_argument( '--qa_home', '--QA_HOME',  dest='QA_HOME',
            help='''Path to the config.txt file and project tables,
~/.qa-dkrz by default. Required for multiple users when there are local
instances of tables.''')

        parser.add_argument('-S', '--select', dest='CL_S',
            help='''Selection of variables, syntax see SELECT.
This overwrites any SELECT assignment.''')

        parser.add_argument('--show_call', '--show-call', dest='SHOW_CALL',
            action="store_true",
            help='''Show C++ executable call, only for debugging.
Number depends on --next''')

        parser.add_argument('--show_conf', '--show-conf', dest='SHOW_CONF',
            action="store_true",
            help="Show the configuration of the current run")

        parser.add_argument('--show_exp', '--show-exps',
            '--SHOW_EXP', '--SHOW_EXPS', dest='SHOW_EXP', action="store_true",
            help='''Show C++ executable call, only for debugging.
Number depends on --next''')

        parser.add_argument('--show_next', '--show-next',
            type=int, nargs='?', default=0, const=1, dest='SHOW_NEXT',
            help="Show C++ executable call(s), only for debugging [next=1].")

        parser.add_argument('-V', '--version', dest='SHOW_VERSION',
            action="store_true",
            help='Display the QA-DKRZ version information.')

        parser.add_argument( '--work', dest='QA_HOME',
            help='''Path to the config.txt file and project tables,
~/.qa-dkrz by default. Required for multiple users using shared tables.''')

        parser.add_argument('NC_FILE', nargs='*',
            help= "NetCDF files [file1.nc[, file2.nc[, ...]]].")


        # the following opts are going to be passed to install
        parser.add_argument( '--auto-up', '--auto_up','--auto-update',
            '--auto_update',
            nargs='?', const='enable', dest='AUTO_UP',
            help="Passed to install")

        parser.add_argument( '--auto-table-up', '--auto_table_up',
            nargs='?', const='enable', dest='AUTO_TABLE_UP',
            help="Passed to install")

        parser.add_argument( '--build',
            action="store_true", dest='BUILD',
            help="Passed to install")

        # the following opts are going to be passed to install
        parser.add_argument( '--link',
            nargs='?', const='t', dest='LINK',
            help="Passed to install")

        parser.add_argument( '--only-qa',
            nargs='?', const='t',
            help="Passed to install")

        parser.add_argument( '--set-default-project', '--set_default_project',
            nargs='?', const='t', dest='SET_DEFAULT_PROJECT',
            help="Passed to install")

        parser.add_argument( '--up', '--update', dest='UPDATE',
            help="Passed to install")

        return parser


    def delOpt(self, key, dct={}):
        if len(dct):
            local_dct = dct
        else:
            local_dct = self.dOpts

        if key in local_dct:
            del local_dct[key]

        return


    def finalSelLoc(self):
        p=[]
        v=[]
        self.dOpts['SELECT_PATH_LIST'] = []
        self.dOpts['SELECT_VAR_LIST']  = []

        for sel in self.lSelect:
            (p,v)=self.getSelLock('S', sel)

            for ix in range(len(p)):
                self.dOpts['SELECT_PATH_LIST'].append(p[ix])
                self.dOpts['SELECT_VAR_LIST'].append(v[ix])

        p=[]
        v=[]
        self.dOpts['LOCK_PATH_LIST'] = []
        self.dOpts['LOCK_VAR_LIST']  = []

        for loc in self.lLock:
            (p,v)=self.getSelLock('L', loc)

            for ix in range(len(p)):
                self.dOpts['LOCK_PATH_LIST'].append(p[ix])
                self.dOpts['LOCK_VAR_LIST'].append(v[ix])

        return


    def getOpt(self, key, dct={}):
        if len(dct):
            local_dct = dct
        else:
            local_dct = self.dOpts

        if key in local_dct:
            return local_dct[key]

        return ''


    def getSelLock(self, key, line):
        # Note for SELECT, LOCK, -S and -L usage:
        # General setting: NAME {space | = | +=}[path[=]][variable]
        #                  already resolved

        # Examples:
        # 1) path=var
        #    specifies a single path where to look for a single variable
        # 2) p1,p2=var
        #    two paths to look for a variable
        # 3) p1,p2=
        #    two paths with every variable, equivalent to p1,p2=.*
        # 4) p1=v1,v2
        #    one path with two variables
        # 5) str1[,str2,str3]
        #    a) no '/' --> variables
        #    b) '/' anywhwere  --> only paths; no trapping of errors
        # 6) relative path without a '/' must have '=' appended
        # 7) p1=,p2=v1...  --> ERROR

        # special: a fully qualified file
        if os.path.isfile(line):
            (h, t)=os.path.split(line)
            if len(h) and len(t):
                line = h + '=' + t
            elif len(h):
                line = h+'='
            else:
                line = t

        if line[0] == '=':
            line = line[1:]
        elif line[0:2] == '+=':
            line = line[2:]

        x_line=line.split('=')

        sz=len(x_line)
        if sz > 2 or sz == 0:
            return # input failure
        elif sz > 1:
            p=x_line[0]
            if len(x_line[1]):
                v=x_line[1]
            else:
                v='.*'
        elif sz == 1:
            if '/' in x_line[0]:
                # no variable
                v='.*'
                p=x_line[0]
            else:
                # no path
                p='.*'
                v=x_line[0]

        # comma-separated items?
        x_p=p.split(',')
        x_v=v.split(',')

        lp=[]
        lv=[]
        for p in x_p:
            for v in x_v:
                lp.append(p)
                lv.append(v)

        return ( lp, lv)


    def isOpt(self, key, inq_bool=False, dct={}):
        if len(dct):
            local_dct = dct
        else:
            local_dct = self.dOpts

        if key in local_dct.keys():
            val = local_dct[key]
            _type = str(type(val))
            if _type.find('bool'):
                return val

            if not inq_bool:
                return True

        return False


    def mergeOptions(self):
        # merge reversed, i.e. from low to high priority
        for ix in range(len(self.ldOpts)-1,-1,-1):
            if len(self.ldOpts[ix]):
                for key in self.ldOpts[ix].keys():
                    if key == 'QA_CONF' and key in self.dOpts:
                        self.dOpts[key] += ',' + self.ldOpts[ix][key]

                    elif key == 'SELECT' and key in self.dOpts:
                        if len(self.dOpts[key]):
                            self.dOpts[key] += ','

                        # SELECT [+=] str: accumulates, SELECT = str: overrules
                        if self.ldOpts[ix][key][0:2] == '+=':
                            self.dOpts[key] += self.ldOpts[ix][key][2:]
                        elif self.ldOpts[ix][key][0] == '=':
                            self.dOpts[key] = self.ldOpts[ix][key][1:]

                    else: # overwrite
                        self.dOpts[key] = self.ldOpts[ix][key]


        return


    def readConfigFiles(self):
        if len(self.ldOpts) == 0:
            self.ldOpts.append({})

        # list config files, if any.
        lCfgFiles=[]

        # At first, any item specified by QA_CONF on the command-line
        if 'QA_CONF' in self.ldOpts[self.cl_ix]:
            lCfgFiles.append(self.ldOpts[self.cl_ix]['QA_CONF'])

        if 'TASK' in self.ldOpts[self.cl_ix]:
            lCfgFiles.append(self.ldOpts[self.cl_ix]['TASK'])

        if len(lCfgFiles) == 0:
            lCfgFiles.append('') # force to enter the for-loop

        for cfg in lCfgFiles:
            self.ldOpts.append({})
            _ldo = self.ldOpts[len(self.ldOpts)-1]

            if len(cfg):
                with open(cfg, 'r') as f:
                    select=''
                    lock=''
                    for line in f:
                        line = qa_util.clear_line(line)
                        if len(line) == 0:
                            continue

                        # SELECT|LOCK may have '=', '+=', or ' '; the latter
                        # two accumulate and the value may have embedded '='
                        if line[:6] == 'SELECT':
                            str0=self.setSelLock(line)
                            if str0[0] == '=':
                                select = str0[1:]
                            else:
                                if len(select):
                                    select += ','
                                select += str0[2:]
                            continue
                        elif line[:4] == 'LOCK':
                            str0=self.setSelLock(line)
                            if str0[0] == '=':
                                lock = str0[1:]
                            else:
                                if len(lock):
                                    lock += ','
                                lock += str0[2:]
                            continue
                        else:
                            pos = line.find('=')
                            if pos == -1:
                                key=line.upper()
                                val='t'
                                if key == 'NEXT':
                                    val=1
                                elif key == 'SHOW_NEXT':
                                    val=1
                            else:
                                (key, val) = line.split('=', 1)
                                key=key.upper()

                        if key == 'PROJECT' and len(self.project) == 0:
                            # for PROJECT_AS, too
                            self.project = val

                        elif len(key) > 2 and key[0:3] == 'QC_':
                            # --> backward compatible
                            key = 'QA' + key[2:]

                        # comma-sep list --> python list, but protect
                        # those containing braces
                        if val.find('{') == -1:
                            try:
                                # --> list of integers
                                val=[ int(x) for x in val.split(',') ]
                            except ValueError:
                            # --> list of strings
                                s=repr(val)
                                if s.find(',') > -1:
                                    val=val.split(',')
                            except:
                                pass

                        _ldo[key] = val  # fill in regular QA options

                    if len(select):
                        self.lSelect.extend(select.split(','))
                    if len(lock):
                        self.lLock.extend(lock.split(','))

            if 'QA_CONF' in _ldo.keys():
                str0 = _ldo['QA_CONF']
                del _ldo['QA_CONF']
            else:
                str0='' # default configuration

            self.searchQA_CONF_chain(str0, lCfgFiles)

        # preserve the list of configuration files
        if len(self.ldOpts):
            val=''
            for cfg in lCfgFiles:
                if len(cfg):
                    if len(val):
                        val += ','
                    val += cfg

            self.ldOpts[0]['QA_CONF'] = val

        return


    def searchQA_CONF_chain(self, fName, lCfgFiles):
        if len(fName) == 0:
            fName=self.project + '_qa.conf'
        elif len(self.project) == 0:
            p0 = fName.find('_')
            if p0 > -1:
                self.project = fName[:p0]
                self.dOpts['PROJECT'] = self.project

        lCfg=[]
        prfx=os.path.join(self.QA_HOME, 'tables')

        lCfg.append(fName)
        lCfg.append(os.path.join(prfx, fName))
        lCfg.append(os.path.join(prfx, self.project, fName))
        lCfg.append(os.path.join(prfx, 'projects', self.project, fName))

        for cfg in lCfg:
            # used before?
            if cfg in lCfgFiles:
                continue

            if os.access(cfg, os.R_OK):
                lCfgFiles.append(cfg)

        return


    def setDefault(self):
        self.ldOpts.append({})
        _ldo = self.ldOpts[len(self.ldOpts)-1]

        _ldo['CFG_FILE']='qa.cfg'
        _ldo['CONDA']=False
        _ldo['HARD_SLEEP_PERIOD']=10
        _ldo['MAIL']='mailx'
        _ldo['NUM_EXEC_THREADS']=1
        _ldo['PROJECT_DATA']='./'
        _ldo['QA_HOST']=socket.gethostname()
        _ldo['QA_RESULTS']=os.getcwd()
        _ldo['REATTEMPT_LIMIT']=5
        _ldo['SLEEP_PERIOD']=300

        _ldo['QA_EXEC_HOSTS']=_ldo['QA_HOST']
        return


    def setHomeDir(self):
        '''
        a) The user has to have write-access to tables.
        b) The tables in QA_SRC may be write-protected (e.g. installed by admins)
        c) Solution: the tables are copied to another location by the user.
            i) The home directory by default.
           ii) A working directory. If i) is not possible or other persons
               should share the same set of tables, then use option --work=path.
               1) there is no tables, yet --> copy from QA_SRC:
               2) there are tables.
        '''

        last=len(self.ldOpts)-1
        if last == -1:
            self.ldOpts.append({})
            last=0

        # note: QA_WORK was mapped to QA_HOME:
        # the place where to look for default tables
        if 'QA_HOME' in self.ldOpts[last]:
            # --work: any tables and/or cfg files?
            targetCfg=os.path.join(self.ldOpts[last]['QA_HOME'], '.qa-dkrz')
            if not os.path.isdir(targetCfg):
                userCfg=os.path.join(os.environ['HOME'], '.qa-dkrz')
                try:
                    shutil.copytree(userCfg, targetCfg)
                except:
                    print 'could not copy ' + userCfg + ' --> ' + targetCfg
                    sys.exit(1)

            self.ldOpts[last]['QA_HOME'] = targetCfg
        else:
            # any tables and/or cfg files?
            targetCfg=os.path.join(os.environ['HOME'], '.qa-dkrz')
            if not os.path.isdir(targetCfg):
                srcCfg= os.path.join(self.qaSrc, '.qa-dkrz')
                try:
                    shutil.copytree(srcCfg, targetCfg)
                except:
                    print 'could not copy ' + srcCfg + ' --> ' + targetCfg
                    sys.exit(1)

        self.QA_HOME = targetCfg
        self.ldOpts[last]['QA_HOME'] = targetCfg

        # a project would identify default tables
        if 'PROJECT_AS' in self.ldOpts[last]:
            self.project = self.ldOpts[last]['PROJECT_AS']
        elif 'PROJECT' in self.ldOpts[last]:
            self.project = self.ldOpts[last]['PROJECT']

        return


    def setSelLock(self, line):
        # process and return line
        if line[0] == 'S':
            line=line[6:]
        elif line[0] == 'L':
            line=line[4:]
        else:
            return line

        p0=0
        while True: # for '=' or '+=' surrounded by blanks
            if line[p0] == '=':
                line=line[p0:]
                break
            elif line[p0:2] == '+=':
                line=line[p0:]
                break
            elif line[p0] != ' ':
                line = '+=' + line[p0:]
                break

            p0 += 1

        return line


class CfgFile(object):
   '''
   classdocs
   # read,write, modify a configuration file
   '''

   def __init__(self, qaOpts):
      '''
      Constructor
      '''
      
      # traditional config file, usually in the users' home directory
      self.rCP = ConfigParser.RawConfigParser()
      self.rCP.optionxform = str # thus, case-sensitive

      self.qaOpts = qaOpts
      self.isModified = False

         
   def entry(self, key='', value=''):

      retVal=value

      # inquire/add from/to an existing cfg files
      try:
         # does section exist?
         getVal =self.rCP.get(self.section, key)
      except ConfigParser.NoSectionError:
         if len(value) and not (value == 'd' or value == 'disable'):
               # a new section
               self.rCP.add_section(self.section)
               self.rCP.set(self.section, key, value)
               self.isModified=True
      except ConfigParser.NoOptionError:
         # a new assignment to an existing section
         if len(value) and not (value == 'd' or value == 'disable'):
               self.rCP.set(self.section, key, value)
               self.isModified=True
               self.qaOpts.addOpt(key,value)
      else:
         # section and key exist
         if len(value):
            if value == 'd' or value == 'disable':
               # delete a key=value pair
               self.rCP.remove_option(self.section, key)
               retVal=''
               self.isModified=True
            elif getVal != value:
               # replacement
               self.rCP.set(self.section, key, value)
               self.isModified=True
         else:
            retVal=getVal

      return retVal


   def read_file(self, file='', section=''):

      self.section = section
      
      if len(file):
         self.file = file
      elif self.qaOpts.isOpt('CFG_FILE'):
         self.file = self.qaOpts.getOpt('CFG_FILE')

      # any valid conf-file written by a ConfigParser?
      (path, tail)=os.path.split(self.file)

      if os.path.isfile(self.file):
         # read a file created by a self.rCP instance
         self.rCP.read(self.file)
         self.isModified=False

      else:
         old = os.path.join(path, 'config.txt' )
           
         if os.path.isfile(old):
         # note config.txt is a file not created  by self.rCP,
         # conversion.
            with open(old) as r:
               self.isModified=True
               
               for line in r:
                  line = qa_util.clear_line(line)
                  if len(line):
                     sz1=len(line)-1

                     if line[sz1] == ':' :
                        ny_sect=line[0:sz1] # rm ':'
                        self.rCP.add_section(ny_sect)
                     else:
                        k, v = line.split('=')
                        self.rCP.set(ny_sect, k, repr(v))

                        
      # append parsed setting to the list of options
      if len(self.section):
         cfg_list = self.rCP.options(self.section)
    
         for key in cfg_list:
            val = self.entry(key=key)
            self.qaOpts.addOpt(key, val)

      return 


   def write_file(self):
   
      if self.isModified:
         with open(self.file,'wb') as cfg:
            self.rCP.write(cfg)
            
      return
