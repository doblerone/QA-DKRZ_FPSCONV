#! /hdh/local/miniconda/bin/python

'''
Created on 21.03.2016

@author: hdh
'''

import sys
import os

import qa_util

class LogSummary(object):
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''

        # defaults
        self.project=''
        self.prj_fName_sep = '_'
        self.prj_path_sep = '/'
        self.prj_var_ix = 0
        self.prj_frq_ix = 1


    def annotation_add(self, path_id, var_id, blk, i):
        # annotation and associated indices of properties
        # fse contains ( var, StartTime, EndTime )
        # index i points to blk[i] == 'caption:'

        # just to have them define in case of an error
        capt=''
        impact=''
        tag=''

        words = blk[i+1].lstrip(' -').split(None,1)
        if words[0] == 'caption:':
            capt = words[1]
            i=i+1

        words = blk[i+1].lstrip(' -').split(None,1)
        if words[0] == 'impact:':
            impact = words[1]
            i=i+1

        words = blk[i+1].lstrip(' -').split(None,1)
        if words[0] == 'tag:':
            tag = words[1]
            j=0
            if tag[0] == "'":
                j=1

            if tag[j:j+2] == 'SF' or tag[j]=='M':
                #  when these kinds of tags appear, there is only a single event
                self.file_count -= 1

            i=i+1

        # the sub-temporal range of a given variable (and path)
        try:
            # index of a found caption
            ix = self.annot_capt.index(capt)
        except:
            # index of a new caption
            ix = len(self.annot_capt)

            self.annot_capt.append(capt)
            self.annot_tag.append(tag)
            self.annot_impact.append(impact)

            self.annot_fName_id.append([var_id])
            self.annot_path_id.append([path_id])

            self.annot_fName_dt_id.append([[self.dt_id]])
        else:
            # a registered annotation

            # associated properties
            try:
                jx = self.annot_path_id[ix].index(path_id)
            except:
                # new path for the current annotation
                # jx = len( self.annot_path_id[ix] )
                self.annot_path_id[ix].append(path_id)

            try:
                jx = self.annot_fName_id[ix].index(var_id)
            except:
                # new variable for the current annotation
                jx = len( self.annot_fName_id[ix] )
                self.annot_fName_id[ix].append(var_id)
                self.annot_fName_dt_id[ix].append([])

            try:
                self.annot_fName_dt_id[ix][jx].index(self.dt_id)
            except:
                # kx = len(self.annot_fName_dt_id[ix][jx])
                self.annot_fName_dt_id[ix][jx].append(self.dt_id)

        j=i

        return j


    def annotation_merge(self):
        # find condensed scopes of annotations

        pItems=[]
        pItems_aix=[]
        pMutables=[]

        annot_sz = len(self.annot_capt)
        p_sz=annot_sz
        
        # there is not a single annotation
        if annot_sz == 0:
            return

        for ix in range(annot_sz):
            # get scope
            a, b = self.annotation_getItems(ix)
            pItems.append(a)
            pMutables.append(b)

            # a, c = self.annotation_getItems(ix, sep='_')

        # across annotations: combine almost identical ones whith a <sub-string>
        # appearing also in the path
        list_simil_ix=[]   # index of similar annotation
        list_simil_jx=[]   # index of path to above
        list_simil_w=[]   # index of path to above

        for ix0 in range(6,annot_sz-1):
            #split at <>
            w0 = qa_util.split(self.annot_capt[ix0], "<>")

            for jx0 in range( len(pItems[ix0]) ):
                simil_ix=[]
                simil_jx=[]
                simil_w=[]
                sz_pI0=len(pItems[ix0][jx0])

                for ix1 in range(ix0+1,annot_sz):
                    #split at <>
                    w1 = qa_util.split(self.annot_capt[ix1], "<>")

                    if len(w0) != len(w1):
                        continue # annotations are not similar

                    for jx1 in range( len(pItems[ix1]) ):
                        isFamily=True
                        sz_pI1=len(pItems[ix1][jx1])

                        # number of paths is identical?
                        if sz_pI0 != sz_pI1:
                            continue

                        for i in range(len(w0)):
                            if w0[i] != w1[i]:
                                for m in range(sz_pI0):
                                    if pItems[ix0][jx0][m] == w0[i] \
                                        and pItems[ix1][jx1][m] == w1[i]:

                                        if len(simil_ix) == 0:
                                            simil_ix.append(ix0)
                                            simil_jx.append(jx0)
                                            simil_w.append(w0[i])
                                        simil_ix.append(ix1)
                                        simil_jx.append(jx1)
                                        simil_w.append(w1[i])
                                        break
                                else:
                                    isFamily=False
                                    break

                    if isFamily and len(simil_ix) > 0:
                        for lst in list_simil_ix:
                            if qa_util.isAmong(simil_ix, lst):
                                break
                        else:
                            list_simil_ix.append(simil_ix)
                            list_simil_jx.append(simil_jx)
                            list_simil_w.append(simil_w)

        for lx in range( len(list_simil_w) ):
            ix0=list_simil_ix[lx][0]

            for ly in range( len(list_simil_w[lx]) ):
                if ly == 0:
                    w_ny=[ list_simil_w[lx][ly] ]
                else:
                    if not list_simil_w[lx][ly] in w_ny:
                        w_ny.append(list_simil_w[lx][ly])

            if len(w_ny):
                s=''
                for w in w_ny:
                    if len(s):
                        s += ','
                    s += w

                # replace <var> by <var,var2,var3,..>
                self.annot_capt[ix0] = self.annot_capt[ix0].replace(\
                    '<'+list_simil_w[lx][0]+'>',\
                    '<' + s + '>')

        lst=[]
        for l in range( len(list_simil_ix) ):
            lst.extend(list_simil_ix[l][1:])

        lst.sort()
        lst.reverse()

        for i in range( len(lst) ):
            ix1=lst[i]
            del self.annot_capt[ix1]
            del self.annot_fName_dt_id[ix1]
            del self.annot_fName_id[ix1]
            del self.annot_impact[ix1]
            del self.annot_path_id[ix1]
            del self.annot_tag[ix1]
            del pItems[ix1]
            del pMutables[ix1]


        # maximal length of path vectors
        sz_kx_max = len(pItems[0][0])
        for i in range(len(pItems)):
            for j in range(len(pItems[i])):
                sz = len(pItems[i][j])
                if sz > sz_kx_max:
                    sz_kx_max = sz

        # maximal length of path vectors within annotations
        sz_jx_max = len(pItems[0])

        '''
        for ix in range(1, annot_sz):
            w0 = qa_util.split(self.annot_capt[ix0], "<>")
            sz = len(self.annot_path_id[ix])
            if sz > sz_jx_max:
                sz_jx_max = sz
        '''

        # some pItems member have no '*', i.e. they represent a single path.
        # Make them also variable, but only if there is any variable pItems object.
        for jx in range(sz_jx_max):
            count=[]
            count_max_ix=0
            count_max=0
            for ix in range(annot_sz):
                if len(pItems[ix]) > jx:
                    count.append(pItems[ix][jx].count('*'))
                    if count[ix] > count_max:
                        count_max=count[ix]
                        count_max_ix = ix

            for ix in range(annot_sz):
                if len(pItems[ix]) > jx:
                    # possible positions of *-pItems; could vary
                    pos=[]
                    for p in range( len(pItems[count_max_ix][jx]) ):
                        if pItems[count_max_ix][jx][p] == '*':
                            pos.append(p)

                    sz_ix = len(pItems[ix][jx])
                    if count[ix] < count_max:
                        for n in pos:
                            if n < sz_ix:
                                pM=pMutables[ix][jx]
                                if pItems[ix][jx][n] != '*':
                                    pM.append(pItems[ix][jx][n])
                                    pItems[ix][jx][n] = '*'
                                    pMutables[ix][jx]=pM


        # get unique paths: step by step
        uniquePaths=[]
        #uniqueMutables=[]
        for ix in range(annot_sz):             # index of annotation items
            uniquePaths.append([])
            #uniqueMutables.append([])

            for p in range( len(pItems[ix]) ):   # index of vectors for given ix
                uniquePaths[ix].append(pItems[ix][p])
                #uniqueMutables[ix].append(pMutables[ix][p])

        # compress identical paths; remember original annotation index in a list
        uniquePaths2=[]
        uniquePaths_ix=[]
        #uniqueMut=[]
        for ix in range(annot_sz):
            for jx in range( len(uniquePaths[ix]) ):
                for nx in range( len(uniquePaths2) ):
                    if uniquePaths[ix][jx] == uniquePaths2[nx]:
                        uniquePaths_ix[nx].append(ix)
                        #uniqueMut[nx].append(uniqueMutables[ix][jx])
                        break
                else:
                    uniquePaths2.append(uniquePaths[ix][jx])
                    uniquePaths_ix.append([ix])
                    #uniqueMut.append(uniqueMutables[ix][jx])
        else:
            uniquePaths=uniquePaths2
            #uniqueMutables=uniqueMut

        # ------
        # begin of writing the JSON files
        for jx in range(len(uniquePaths)):
            #f0_ix=uniquePaths_ix[jx][px]

            name = self.getPathPrefix(uniquePaths[jx]) #, pItems[f0_ix][jx])
            fl = os.path.join(self.f_annot, name + '.json')

            with open(fl, 'w') as fd:
                self.write_json_header(uniquePaths[jx], fd)

                # write annotations for given path prefixes or a kind of MIP name
                isInit=True

                for i in range( len(uniquePaths_ix[jx]) ):
                    ix = uniquePaths_ix[jx][i]

                    self.write_json_annot(ix, jx,
                            pItems[ix], pMutables[ix],
                            fd, init=isInit )

                    isInit=False

                self.write_json_annot(0, 0,
                    pItems[0], pMutables[ix],
                    fd, final_annot=True, final=True )

        '''
        # write a file with annotation, path and file name for each tag
        for ix in range(len(self.annot_tag)):
            # basePath = self.getPathPrefix(uniquePaths[jx], pItems[f0_ix][jx])
            tag = self.annot_tag[ix].strip("'")

            name = self.annot_impact[ix] + '_'+ tag
            fl = os.path.join(self.tag_dir, tag)

            with open(fl, 'w') as fd:
                self.write_tag_file(ix, tag, uniquePaths,
                    pItems[ix],
                    fd)
        '''

        return


    def annotation_reset(self, var_id):
        # resets for a particular atomic variable

        for ix in range(len(self.annot_capt)-1,-1,-1):
            try:
                jx = self.annot_fName_id[ix].index(var_id)
            except:
                continue
            else:
                # there is something to clear.
                del self.annot_fName_dt_id[ix][jx]
                del self.annot_fName_id[ix][jx]
                del self.annot_path_id[ix][jx]

        return


    def annotation_getItems(self, ix, sep='/'):
        # ix: index of annotation
        # self.prj_var_ix: path with variable as i-th component
        # self.prj_frq_ix: path with frequency as i-th component

        # find identical indices of path and variable items
        # within the annot_..._ids

        # Find the beginning of a path common for the entire annotation.
        # Note that each index of p_items stores a list of corresponding
        # path components.

        # init of pItems[[]] found for this, i.e. ix-th, annotation.
        # Mutable items are represented by '*'
        # pItems takes into account the possibility that different annotation
        # paths are of different size. pItems_api[[]] gives corresponding
        # api elements.
        if sep == '/':
            api = self.annot_path_id[ix]
            items, mutables = self.getAnnotSource(api)
        else:
            afi = self.annot_fName_id[ix]
            items, mutables = self.getAnnotSource(afi, sep='_')

        return items, mutables


    def check_for_skipping(self, blk, skip_fBase):
        for line in blk:
            x_line = line.split()
            if x_line[0] == 'file:':
                for f in skip_fBase:
                    if f+'.nc' == x_line[1] or f == x_line[1]:
                        blk = []
                        return

                break

        return


    def decomposition(self, name, items, lst_ids, sep):
        # Decompose a filename or a path.
        if name[-3:] == '.nc':
            name=name[0:-3]

        itm_id=[]

        x_name = name.split(sep)
        sz = len(x_name)

        # a variable special: discard trailing date-range
        dtr = self.dt[self.dt_id]
        if dtr == x_name[sz-1]:
            sz -= 1

        # members of a the DRS components
        if sep == self.prj_path_sep:
            # init
            if len(self.p_drs) == 0:
                for i in range(sz):
                    self.p_drs.append([])

            # update
            for i in range(sz):
                if not x_name[i] in self.p_drs[i]:
                    self.p_drs[i].append(x_name[i])

        for i in range(sz):
            # update the list of var or path items, respectively
            try:
                itm_id.append( items.index(x_name[i]) )
            except:
                itm_id.append(len(items))
                items.append(x_name[i])

        try:
            ID = lst_ids.index(itm_id)
        except:
            ID = len(lst_ids)
            lst_ids.append(itm_id)

        return ID


    def getAnnotSource(self, a_item_ix, sep='/'):

        # references
        if sep == '/':
            # basic path
            ref_ids = self.path_ids
            ref_items = self.p_items
        else:
            # basic filename
            ref_ids = self.fName_ids
            ref_items = self.f_items

        sz_max=len(ref_ids[a_item_ix[0]])
        for i in range(1, len(a_item_ix)):
            sz = len(ref_ids[a_item_ix[i]])
            if sz > sz_max:
                sz_max = sz

        same=[]
        ref_id_0 = ref_ids[a_item_ix[0]]
        for i in range(sz_max):
            same.append(True)

            for j in range(len(a_item_ix)):
                aix = a_item_ix[j]
                sz = len(ref_ids[aix])

                if i == sz:
                    break

                ref_id = ref_ids[aix]

                if ref_items[ref_id_0[i]] != ref_items[ref_id[i]]:
                    same[i]=False
                    break

        items=[]
        mutables=[]

        for i in range(len(a_item_ix)):
            aix=a_item_ix[i]
            itm=[]
            mut=[]
            for j in range(len(ref_ids[aix])):
                k = ref_ids[aix][j]

                if not same[j]:
                    itm.append('*')
                    mut.append(ref_items[k])
                else:
                    itm.append(ref_items[k])

            isNewItm=True
            for j in range(len(items)):
                if itm == items[j]:
                    isNewItm=False
                    break

            mutables.append(mut)

            if isNewItm:
                items.append(itm)


        if sep == '/':
            try:
                self.pathBase
                for jx in range(len(items)):
                    count=0
                    for i in range(len(items[jx])):
                        if items[jx][i] == self.pathBase:
                            for j in range(i):
                                del items[jx][0]
                                count += 1
                            break
            except:
                pass

        return items, mutables


    def get_next_blk(self, fd, f_log='', get_prmbl=False, skip_fBase=[]):
        # read the next entry from a log-file or alternatively from provided fd

        blk = []
        if len(skip_fBase):
            is_skip_fBase=True
        else:
            is_skip_fBase=False

        # read preamble when the file was just opened
        if fd.tell() == 0:
            self.blk_last_line=''

            # items from the preamble
            for line in fd:
               line = line.rstrip(' \n')
               blk.append(line)

               words = line.split(None, 1)
               
               if words[0] == 'items:':
                  self.blk_last_line = fd.next().rstrip(' \n')
                  break

               # a few opts are used by this script
               self.get_preamble_opt(words)

            # for logPathIndex, pathBase has also to be defined
            try:
                self.logPathIndex
            except:
                pass
            else:
                try:
                    self.pathBase
                except:
                    del self.logPathIndex

            if get_prmbl:
                return blk
            else:
                blk = []

        if len(self.blk_last_line):
            blk.append(self.blk_last_line)

        for line in fd:
            line = line.rstrip(' \n')
            if len(line) == 0:
                continue

            if '- date:' in line:
                # eventually for the current block
                if len(blk):
                    # blk is reset for skip condition True
                    if is_skip_fBase:
                        self.check_for_skipping(blk, skip_fBase)

                    # Preserve the current line for next blk.
                    self.blk_last_line = line
                    return blk

            blk.append(line)
        else:
            # the last entry was just read
            self.blk_last_line = ''

        # finalise after EOF.
        # If blk length > 0, then check the current blk,
        # which wasn't done in the loop

        # Another call would have blk = [].
        if is_skip_fBase:
            self.check_for_skipping(blk, skip_fBase)

        return blk


    def getPathPrefix(self, path): #, astItems):
        # Default: turn a path into '_' separated string.
        # If given, use a project provided identification string instead.
        # If available, use LOG_PATH_INDEX for assembling a unique name

#        path = prefix
#        path.extend(astItems)
#        if len(path[0]) == 0:
#            del path[0]

        s=''
        sz = len(path)

        try:
            currIx = self.logPathIndex
        except:
            currIx = range(len(path))

        for i in range(len(currIx)):
            ix = int(currIx[i])
            if ix < sz and path[ix] != '*':
                if len(s) > 0:
                    s += '_'
                s += path[ix]

        return s


    def period_add(self, var_id, path_id, fse, blk, i):
        # Requires old-to-young sorted sub-temporal files of an atomic variable
        # the procedure takes a missing 'begin' but present 'end' into account;
        # also missing of the two. Should never happen.

        beg = blk[i+1].split()
        if beg[0] == 'begin:':
            i=i+1
            if len(self.atomicBeg[var_id]) == 0:
                self.atomicBeg[var_id] = beg[1]

        end = blk[i+1].split()
        if end[0] == 'end:':
            i=i+1
            if end[1] > self.atomicEnd[var_id] :
                self.atomicEnd[var_id] = end[1]
            else:
                # suspecting a reset of an atomic variable
                self.annotation_reset(var_id)

                self.atomicBeg[var_id] = beg[1]
                self.atomicEnd[var_id] = end[1]

        return i


    def period_final(self, log_name):
        # find indices for the different frequencies,
        # note that these are dependent on the project

        frqs=[]
        frqs_id=[] # this will be expanded to a 2D array
        frqs_beg=[] # this will be expanded to a 2D array
        frqs_end=[] # this will be expanded to a 2D array
        sz_max=0

        for i in range(len(self.fName_ids)-1):
            #if self.project == 'CORDEX':
            name = self.reassemble_name(self.f_items,
                                        self.fName_ids[i],
                                        self.prj_fName_sep)
            sz = len(name)
            if sz > sz_max:
                sz_max = sz

            word = name.rsplit('_', 2)
            for j in range(len(frqs)):
                if word[2] == frqs[j]:
                    frqs_id[j].append(i)
                    break
            else:
                # append new lists
                frqs.append(word[2])
                frqs_id.append([i])
                frqs_beg.append(self.atomicBeg[i])
                frqs_end.append(self.atomicEnd[i])

        # find the most extended begin and end, respectively, for each frequency
        for j in range(len(frqs)):
            ix = frqs_id[j]
            for i in range(1, len(ix)):
                if frqs_beg[j] > self.atomicBeg[ix[i]]:
                    frqs_beg[j] = self.atomicBeg[ix[i]]

                if frqs_end[j] < self.atomicEnd[ix[i]]:
                    frqs_end[j] = self.atomicEnd[ix[i]]

        # yaml output to a file in directory 'Period'
        f_period  = os.path.join(self.f_period, log_name + '.period')

        with open(f_period, 'w') as fd:
            fd.write("--- # Time intervals of atomic variables.\n")

            for j in range(len(frqs)):
                line =  '\n- frequency: ' + frqs[j] + '\n'
                line += '  number_of_variables: ' + repr(len(frqs_id[j]))
                fd.write(line + '\n')

                for k in range(len(frqs_id[j])):
                    ix = frqs_id[j][k]

                    line  = '  - variable: '
                    line += self.reassemble_name(self.f_items,
                                                      self.fName_ids[ix],
                                                      self.prj_fName_sep) + '\n'

                    if self.atomicBeg[ix] == '':
                        line += '    begin:\n'
                    else:
                        line += '    begin: ' + self.atomicBeg[ix] + '\n'

                    line += '    end: ' + self.atomicEnd[ix] + '\n'

                    status = ''
                    if frqs_beg[j] !=  self.atomicBeg[ix]:
                        status += 'B\n'
                    if frqs_end[j] !=  self.atomicEnd[ix]:
                        status += 'E\n'

                    if len(status):
                        line += '    status: FAIL:' + status
                    else:
                        line += '    status: PASS\n'

                    fd.write(line)

        # human-readable output into Summary
        hyphen     = ' - '
        mark_ok    = '    '
        mark_left  = '--> '
        mark_right = ' <--'
        sz_max += 1

        self.f_range  = os.path.join(self.f_period, log_name + '.range')
        with open(self.f_range, 'w') as fd:
            fd.write("# Time intervals of atomic variables.\n")

            for j in range(len(frqs)):
                line =  '\nFrequency: ' + frqs[j] + ','
                line += ' number of variables: ' + repr(len(frqs_id[j]))
                fd.write(line + '\n')

                for k in range(len(frqs_id[j])):
                    ix = frqs_id[j][k]

                    line  = self.reassemble_name(self.f_items,
                                                 self.fName_ids[ix],
                                                 self.prj_fName_sep).ljust(sz_max)

                    if frqs_beg[j] !=  self.atomicBeg[ix]:
                        line += mark_left
                    else:
                        line += mark_ok

                    line += self.atomicBeg[ix] + hyphen
                    line += self.atomicEnd[ix]

                    if frqs_end[j] !=  self.atomicEnd[ix]:
                        line += mark_right
                    else:
                        line += mark_ok

                    fd.write(line + '\n')

        return


    def get_preamble_opt(self, words):
         if words[0] == 'PROJECT:' and len(self.project) == 0:
            self.project = words[1]
         elif words[0] == 'FILE_NAME_SEP':
            self.prj_fName_sep = words[1]
         elif words[0] == 'PATH_SEP':
            self.prj_path_sep = words[1]
         elif words[0] == 'FILE_NAME_VAR_INDEX':
            self.prj_var_ix = words[1]
         elif words[0] == 'FILE_NAME_FREQ_INDEX':
            self.prj_frq_ix = words[1]            
         elif words[0] == 'DRS_PATH_BASE:' \
               or words[0] == 'LOG_PATH_BASE:' \
                     or words[0] == 'EXP_PATH_BASE:':
            self.pathBase = words[1]
         elif words[0] == 'LOG_PATH_INDEX:' \
               or words[0] == 'EXP_PATH_INDEX:':
            # the following does not seem to work, although it should
            # self.logPathIndex = words[1].split('[, ]')
            #so, a work-around
            w2=words[1].replace('[','')
            w2=w2.replace(']','')
            self.logPathIndex = w2.split(', ')

         return
      
    def prelude(self, f_log):
        if repr(type(f_log)).find('str') > -1:
            f_log = [f_log]

        # would be sufficient to provide only one file with a path or
        # even only a path
        coll=[]
        for i in range( len(f_log) ):
            if os.path.isdir(f_log[i]):
                head = f_log[i]
            else:
                if f_log[i][-4:] != '.log':
                    f_log[i] = f_log[i] + '.log'

                # look for a path component
                head, tail = os.path.split(f_log[i])

                if len(head) == 0:
                    coll.append(i)

        # could be empty
        for i in coll:
            f_log[i] = os.path.join(head, f_log[i])

        return f_log  # always a list


    def reassemble_name(self, items, lst_ix, sep):
        # Assemble the name from the components, collected in lst_ix (path ar var)
        # Note that for a path the leading '/' is represented by an empty list item,
        # which is here reassembled neatly.

        sz = len(lst_ix)
        name=''

        if sz:
            name = items[lst_ix[0]]

            for i in range(1, sz):
                name += sep + items[lst_ix[i]]

        return name


    def run(self, f_log):
        # extraction of annotations and atomic time ranges from log-files
        log_path, log_name = os.path.split(f_log)
        log_name = log_name[0:-4]

        # sub-directories in check_logs
        self.f_annot = os.path.join(log_path, 'Annotations')
        self.f_period  = os.path.join(log_path, 'Period')
        self.tag_dir = os.path.join(log_path, 'Tags', log_name)

        qa_util.mkdirP(self.f_annot)
        qa_util.mkdirP(self.f_period)
        #qa_util.mkdirP(self.tag_dir)

        # time range of atomic variables; in order to save mem,
        # beg and end, respectively, are linked to the name by the
        # index of the corresponding atomic variable name in var
        self.fName_ids=[]
        self.fName_dt_id={}  # each fName_id gets a list of dt ids
        self.path_ids=[]
        self.f_items=[]
        self.p_items=[]
        self.p_drs=[]

        self.var_ids={}  # contains all [ids] with the same variable name in {}

        self.atomicBeg=[]         # atomic time interval
        self.atomicEnd=[]         # atomic time interval
        self.dt=[]                # time intervals of sub-temp files

        self.annot_capt=[]        # brief annotations
        self.annot_impact=[]      # corresponding  severity level
        self.annot_tag=[]         # corresponding tag
        self.annot_scope=[]       # brief annotations
        self.annot_fName_id=[]    # for each var involved
        self.annot_path_id=[]  
        self.annot_var_ix=[]      # only project variable names
        self.annot_fName_dt_id=[] # for each time interval of each var involved

        # count total occurrences (good and bad)
        self.file_count=0
        self.var_dt_count=[]  # for all frequencies

        # reading and processing of the logfile
        with open(f_log, 'r') as fd:
            while True:
                # read the lines of the next check
                blk = self.get_next_blk(fd=fd)
                sz = len(blk) - 1
                if sz == -1:
                    break

                isMissPeriod=True  # fx or segmentation fault

                i=-1
                while i < sz :
                    i = i+1
                    words = blk[i].lstrip(' -').split(None,1)
                    if len(words) == 0:
                        # a string of just '-' would results in this
                        words=['-----------']
                       
                    if words[0] == 'file:':
                        # fse contains ( var, StartTime, EndTime ); the
                        # times could be empty strings or EndTime could be empty
                        fse = qa_util.f_time_range(words[1])
                        self.set_curr_dt(fse)
                        fName_id_ix = self.decomposition(words[1], self.f_items,
                                                    self.fName_ids, self.prj_fName_sep)
                        self.file_count += 1

                        # for counting atomic variable's sub_temps for all freqs
                        try:
                            self.fName_dt_id[fName_id_ix]
                        except:
                            self.fName_dt_id[fName_id_ix] = [self.dt_id]
                        else:
                            self.fName_dt_id[fName_id_ix].append(self.dt_id)

                        try:
                            vName = self.f_items[self.fName_ids[fName_id_ix][self.prj_var_ix]]
                        except:
                            pass
                        else:
                            # dict of varNames to contained var_ids
                            try:
                                self.var_ids[vName]
                            except:
                                self.var_ids[vName] = [fName_id_ix]
                            else:
                                try:
                                    self.var_ids[vName].index(fName_id_ix)
                                except:
                                    self.var_ids[vName].append(fName_id_ix)


                        if fName_id_ix > len(self.atomicBeg) - 1 :
                            # init for a new variable
                            self.atomicBeg.append('')  # greater than any date
                            self.atomicEnd.append('')   # smaller than any date

                    elif words[0] == 'data_path:':
                        # used later
                        path_id = self.decomposition(words[1], self.p_items,
                                                     self.path_ids, self.prj_path_sep)

                    elif words[0] == 'period:':
                        # time ranges of atomic variables
                        # indexed by self.curr_dt within the function
                        i = self.period_add(fName_id_ix, path_id, fse, blk, i)
                        isMissPeriod=False

                    elif words[0] == 'event:':
                        if isMissPeriod and len(fse[2]):
                            self.subst_period(fName_id_ix, path_id, fse)
                            isMissPeriod=False

                        # annotation and associated indices of properties
                        i = self.annotation_add(path_id, fName_id_ix, blk, i)

                    elif words[0] == 'status:':
                        if isMissPeriod and len(fse[2]):
                            self.subst_period(fName_id_ix, path_id, fse)

        if self.file_count == 0:
            return

        # test for ragged time intervals of atomic variables for given frequency
        self.period_final(log_name)

        self.annotation_merge()

        self.sendMail()

        return


    def sendMail(self):
      return


    def set_curr_dt(self, fse):
        dt = ''
        if len(fse[1]):
            dt = fse[1]
            if len(fse[2]):
                dt += '-' + fse[2]

        try:
            self.dt_id = self.dt.index(dt)
        except:
            self.dt_id = len(self.dt)
            self.dt.append(dt)

        return


    def subst_period(self, var_id, path_id, fse):
        # convert DRS date to real date
        tmplt = '0000-01-01T00:00:00'
        dt = [ tmplt, tmplt ]

        for i in range(2):
            j=i+1

            dt[i] = fse[j][0:4].rjust(4,'0') + dt[i][4:]
            if len(fse[1]) > 4:
                dt[i] = dt[i][0:5] + fse[j][4:6].rjust(2,'0') + dt[i][7:]

                if len(fse[j]) > 6:
                    dt[i] = dt[i][0:8] + fse[j][6:8].rjust(2,'0') + dt[i][10:]

                    if len(fse[j]) > 8:
                        dt[i] = dt[i][0:11] + fse[j][8:10].rjust(2,'0') + dt[i][13:]

                        if len(fse[j]) > 10:
                            dt[i] = dt[i][0:14] + fse[j][10:12].rjust(2,'0') + dt[i][16:]

                            if len(fse[1]) > 12:
                                dt[i] = dt[i][0:17] + fse[j][12:4].rjust(2,'0')


        t_blk=['period:']
        t_blk.append('begin: ' + dt[0])
        t_blk.append('end: ' + dt[1])

        fse = ( fse[0], dt[0], dt[1] )

        self.period_add(var_id, path_id, fse, t_blk, 0)

        return


    def write_json_header(self, path, fd):
        # write header of QA results

        tab='    '

        qa_cnclsn=''

        fd.write('{\n')
        fd.write(tab + '"QA_conclusion": "' + qa_cnclsn + '",\n')
        fd.write(tab + '"project": "' +  self.project + '",\n')

        j=-1

        '''
        # remedy when a member of uniquePaths is also in pItems
        raPath=[]
        notAppended=True
        for i in range( len(path) ):
            pP = path[i][px]
            if pP in pItems:
                for pI in pItems:
                    raPath.append(pI)

                notAppended=False
                break
            else:
                raPath.append(pP)

        if notAppended:
            for pI in pItems:
                raPath.append(pI)
        '''

        self.shared_DRS=[]
        for i in range(len(path)):
            if len(path[i]) > 0:
                s='"DRS_' + str(i)
                fd.write(tab + s + '": "')

                if path[i] == '*':
                    self.shared_DRS.append(s)
                    fd.write('SHARED",\n')
                else:
                    fd.write(path[i] + '",\n')

        return


    def write_json_annot(self, ix, jx,
                            pItems, mutables,
                            fd, init=False, final=False, final_annot=False):
        # write annotation
        tab='    '

        keys=[]

        if final_annot:
            fd.write(tab + '\n' + tab +']\n')
            if final:
                fd.write('}\n')

            return

        if final:
            fd.write('}\n')
            return

        if init:
            fd.write(tab + '"annotation":\n')
            fd.write(tab + '[\n')
            fd.write(2*tab + '{\n')
        else:
            fd.write(',\n' + 2*tab + '{\n')

        # verify that all mutables at given ix have the same lengths
        sz_min=len(mutables[0])
        sz_max=sz_min

        for k in range(1,len(mutables)):
            sz=len(mutables[k])
            if sz < sz_min:
                sz_min = sz
            if sz > sz_max:
                sz_max = sz

        sz = len( mutables )
        if sz_min == sz_max:
            for k in range(sz_min):
                unique=[]
                for l in range(sz):
                    for m in range(len(unique)):
                        if unique[m] == mutables[l][k]:
                            break
                    else:
                        unique.append(mutables[l][k])

                fd.write(3*tab + self.shared_DRS[k] + '": ')

                keys=''
                for m in unique:
                    if len(keys):
                        keys +=', '
                    keys += '"' + m + '"'

                fd.write(keys + ',\n')

        capt = self.annot_capt[ix].strip("'")
        fd.write(3*tab + '"caption": "' + capt + '",\n')

        impact = self.annot_impact[ix].strip("'")
        fd.write(3*tab + '"severity": "' + impact + '"\n')
        fd.write(2*tab + '}')

        return


    def write_tag_file(self, ix,  tag, uniquePaths,
                            pItems,
                            fd):
        # write annotation files for tag[ix]

        capt = self.annot_capt[ix].strip("'")
        impact = self.annot_impact[ix].strip("'")
        #tag = self.annot_tag[ix].strip("'")

        fd.write('annotation: ' + capt + '\n')
        fd.write('impact:     ' + impact + '\n')
        fd.write('tag:        ' + tag + '\n')

        return


if __name__ == '__main__':
    summary = LogSummary()
    f_logs = summary.prelude(sys.argv[1:])

    for f_log in f_logs:
        summary.run(f_log) # prelude returns always a list
