'''
Created on 21.03.2016

str     def clearLine(line)
str     def date(t=0):
str     def getQA_SRC(path):
bool    def isValidVarName(name):
bool    def mkdirP(path):
str     def strDelChar(str0, delim=' '):
str     def stripFirstItem(str0, sep='/', item=''):
str     def stripLastItem(str0, sep='/', item=''):
str     def strFirstItem(str0, sep='/'):
str     def strLastItem(str0, sep='/'):

@author: hdh
'''

import sys
import os
import shutil
import subprocess
import re
import keyword
import time

import ConfigParser

def cat( srcs, dest):
    # convert a string into a list
    if repr(type(srcs)).find('str') > -1:
        srcs=[srcs]

    # check whether on of src equals dest
    isNameClash=False
    for src in srcs:
        if src == dest:
            isNameClash=True
            origDest=dest
            dest += repr(os.getpid())
            break

    with open(dest, 'w') as w:
        for src in srcs:
            with open(src) as r:
                for line in r:
                    w.write(line)

    if isNameClash:
        os.rename(dest, origDest)

    return


def cfgParser(rCP, cfgFile, section='', key='', value='', init=False, final=False):
    hasChanged=False

    # any valid conf-file written by a ConfigParser?
    if init:
        path=stripLastItem(cfgFile, '/')
        old = os.path.join(path, 'config.txt' )

        if os.path.isfile(cfgFile):
            # read a file created by a rCP instance
            rCP.read(cfgFile)

        elif os.path.isfile(old):
            # note config.txt is a file not created  by rCP,
            # conversion.
            with open(old) as r:
                for line in r:
                    line = clearLine(line)
                    if len(line):
                        sz1=len(line)-1

                        if line[sz1] == ':' :
                            ny_sect=line[0:sz1] # rm ':'
                            rCP.add_section(ny_sect)
                        else:
                            k, v = line.split('=')
                            rCP.set(ny_sect, k, repr(v))

                        hasChanged=True
        return

    if final == True and hasChanged == True:
        with open(cfgFile,'wb') as cfg:
            rCP.write(cfg)

    # compare+adjust, or set
    bVal=False
    if len(value):
        bVal=True

    retVal=value

    # inquire/add from/to an existing cfg files
    try:
        # does section exist?
        getVal =rCP.get(section, key)
    except ConfigParser.NoSectionError:
        if bVal and not ( value == 'd' or value == 'disable'):
            # a new section
            rCP.add_section(section)
            if len(value):
                rCP.set(section, key, value)
            hasChanged=True
    except ConfigParser.NoOptionError:
        # a new assignment to an existing section
        if bVal and not (value == 'd' or value == 'disable'):
            rCP.set(section, key, value)
            hasChanged=True
    else:
        # try: section and key exist
        if bVal:
            if value == 'd' or value == 'disable':
                # delete a key=value pair
                rCP.remove_option(section, key)
                retVal=''
                hasChanged=True
            elif getVal != value:
                # replacement
                rCP.set(section, key, value)
                hasChanged=True

    return retVal


def clearLine(line):
    line = line.strip() # white chars and \n

    pos = line.find('#')
    if pos > -1:
        line = line[0:pos]

    return line


def date(t=0):
    if t == 0:
        lt = time.localtime(time.time())

        # ISO
        dt = '{}-{}-{}T{}:{}:{}'.format(lt[0], \
            str(lt[1]).rjust(2,'0'), \
            str(lt[2]).rjust(2,'0'), \
            str(lt[3]).rjust(2,'0'), \
            str(lt[4]).rjust(2,'0'), \
            str(lt[5]).rjust(2,'0') )

    return dt

def fGetModTime(f):
    try:
        st=os.stat(f)
    except OSError:
#        print 'qa_util.fGetModTime(' + f + '): no such file'
        t=0
    else:
        t = st.st_mtime

    return t


def fStrReplace(rFile, items, repls):
    # items: list or str
    # repls: list or str. If the number is identical, the used by pairs
    #        If number of items > 1 and number of repls == 1, then replace each
    #        item by repls[0]

    if repr(type(items)).find('str') > -1:
        items=[ items ]

    num=len(items)

    if repr(type(repls)).find('str') > -1:
        rpls=[repls]
        i=1
        while i < num:
            rpls.append(repls[0])
        repls=rpls

    if num != len(repls):
        print 'inappropriate args items=' + repr(items) + ' and repl=' + repr(repls)

    d_lenItems=[]
    for itm in items:
        d_lenItems.append(len(itm))  # length of strings

    rng = range(len(d_lenItems))
    wFile = rFile + repr(os.getpid())

    with open(rFile, 'r') as f:
        try:
            w = open(wFile, 'w')
        except IOError:
            print 'could not open file ' + wFile
            sys.exit(1)

        for line in f:
            line = clearLine(line)
            if len(line) == 0:
                continue

            for ix in rng: # loop over items
                p=-1
                while True: # loop over multiple occurrences per line
                    p=line.find(items[ix], p+1)
                    if p == -1:
                        break
                    else:
                        line0=line[0:p]
                        line1=line[p+d_lenItems[ix]:]
                        line = line0 + repls[ix] + line1

            w.write(line+'\n')


        w.close()
        os.rename(wFile, rFile)

        return


def getCurrRevision(src, is_CONDA):
    d={}

    if is_CONDA:
        with open( os.path.join(src,'install.log')) as f:
            lst=f.read().split()

        for l in lst:
            t0,t1=(l.split('='))
            d[t0]=t1
    else:
        # inquire the git branch
        try:
            lst=subprocess.check_output(["git",
                                          "--work-tree="+src,
                                          "--git-dir="+src+"/.git",
                                          "branch"
                                          ]).split('\n')
        except subprocess.CalledProcessError as e:
            print e.output()
            d['branch']='?'
            d['hexa']='?'
            return ''
        else:
            for s in lst:
                if s[0] == '*':
                    s = s.split()
                    d['branch'] = s[1]
                    break

        # inquire the git version of the branch
        try:
            s=subprocess.check_output(["git",
                                          "--work-tree="+src,
                                          "--git-dir="+src+"/.git",
                                          "log",
                                          "--pretty=format:'%h'",
                                          "-n",
                                          "1"
                                          ])
        except subprocess.CalledProcessError as e:
            print e.output()
            d['branch']='?'
            d['hexa']='?'
            return ''
        else:
            d['hexa'] = s

    return d['branch'] + '-' + d['hexa']


def getQA_SRC(path):
    p = stripLastItem(path)

    if os.access( os.path.join(stripLastItem(p), 'conda-meta'), os.F_OK):
        return (True, os.path.join(p, 'qa-dkrz') )  # true: installed by CONDA

    target=''

    if path[0] == '/':
        target = path
    else:
        target=os.getcwd()

    if os.path.islink(target):
        link=os.readlink(target)

        # link is relative, so make it absolute
        if link[0] != '/':
            link = os.path.join(os.path.dirname(target),link)

        getQA_SRC(link)
    elif os.access(target, os.R_OK):
        # a real instance, at first resolve .. and .
        # works also for . or .. in the middle of the path
        target = stripLastItem(target)

        if target[0] == '/':
            arr=target[1:].split('/')
        else:
            arr=target.split('/')

        for i in range(1,len(arr)):
            if arr[i] == '.':
                arr[i]=''
            elif arr[i] == '..':
                j = i-1
                while not len(arr[j]):
                    j -= 1

                arr[i]=''
                arr[j]=''

        # get rid of empty items
        brr=[]
        for i in range(0,len(arr)):
            if len(arr[i]):
                brr.append(arr[i])

        tmp=''
        count=0
        for i in range(0,len(brr)):
            count+=1
            tmp += '/' + brr[i]

            if os.path.isfile(tmp + '/.install_configure'):
                return (False, tmp)

        if count == len(brr):
            isInvalid=True
    else:
        isInvalid=True

    if isInvalid:
        if os.path.islink(target):
            print 'broken link ' + path
        else:
            print 'invalid path=' + path

        sys.exit(1)

    return (False, '')


def isValidVarName(name):
    # test for a valid variable name
    name = re.sub('[^0-9a-zA-Z_]', '', name)  # rm invalid chars
    name = re.sub('^[^a-zA-Z_]+', '', name)   # rm invalid leading chars

    if not len(name):
        return False

    if keyword.iskeyword(name):
        return False

    return True


def mkdirP(pth):

    # os.makedirs raises error when dirs already exist
    try:
        os.makedirs(pth)
    except OSError:
        pass

    # test for a genuine failed making of dirs
    if not os.path.isdir(pth):
        os.makedirs(pth)
        return False

    return True

def rmdirP(*args):
    status=True
    for a in args:
        if os.path.isdir(a):
            for rs, ds, fs in os.walk(a):
                if len(fs):
                    try:
                        os.removedirs(rs)
                    except:
                        pass

    return


def rmR(*args):
    status=True
    for a in args:
        if os.path.isdir(a) or os.path.isfile(a):
            try:
                os.remove(a)
            except OSError:
                shutil.rmtree(a)
            except:
                pass

    return


def strDelChar(str0, delim=' '):
    if str0.find(delim) == -1:
        return str0

    str1=''
    for p in range(len(str0)):
        if str0[p] != delim:
            str1 += str0[p]

    return str1


def stripFirstItem(str0, sep='/', item=''):
    # only strip if string 'only' is available
    if len(item):
        if item.find(sep) == -1:
            item += sep

        if item.find(item) == -1:
            return str0

    return str0[str0.find(sep)+1:]


def stripLastItem(str0, sep='/', item=''):
    # item strip if string 'item' is available
    if len(item):
        if item.find(sep) == -1:
            item = sep+item

        if item.rfind(item) == -1:
            return str0

    return str0[0:str0.rfind(sep)]


def strFirstItem(str0, sep='/'):
    return str0[0:str0.find(sep)]


def strLastItem(str0, sep='/'):
    return str0[str0.rfind(sep)+1:]


def updateTables():
    return


def which(items):
    # items: list or str
    if repr(type(items)).find('str') > -1:
        items=[ items ]

    # note that the return value is strictly valid only for a single item
    b=False

    with open(os.devnull, 'w') as DEV_NULL:
        for item in items:
            if subprocess.call(["which", item]
                            , stdout=DEV_NULL, stderr=DEV_NULL ) == 0:
                b=True
            else:
                print "which: no " + item + "in PATH"

    return b
