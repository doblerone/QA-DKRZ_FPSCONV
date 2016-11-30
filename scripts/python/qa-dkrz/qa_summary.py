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

        # init of available projects
        self.isCORDEX=False
        self.isCMIP=False
        self.project=''
        self.prj_var_ix = -1
        self.prj_frq_ix = -1
        self.pathBase=''


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
            self.annot_tag.append(impact + '_' + tag)

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
        ampNames_ix=[]
        fItems=[]
        fItems_aix=[]
        amfNames_ix=[]

        annot_sz = len(self.annot_capt)

        for ix in range(annot_sz):
            # get scope
            a, b = self.annotation_getItems(ix)
            pItems.append(a)
            pItems_aix.append(b)

            a, b = self.annotation_getItems(ix, sep='_')
            fItems.append(a)
            fItems_aix.append(b)

        # different path structures result in multi-dimensionality
        sz_jx_max = len(pItems[0])
        sz_jx_max_ix=0

        for ix in range(annot_sz):
            if len(pItems[ix]) > sz_jx_max:
                sz_jx_max = len(pItems[ix])
                sz_jx_max_ix=ix

        # some pItems member have no '*', i.e. they represent a single path.
        # Make them also variable
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
                    if count[ix] < count_max:
                        for n in range(len(pItems[ix][jx])):
                            if pItems[count_max_ix][jx][n] == '*':
                                pItems[ix][jx][n] = '*'



        for ix in range(annot_sz):
            a = self.annotation_amNames(ix, pItems[ix], pItems_aix[ix] )
            ampNames_ix.append(a)

            a = self.annotation_amNames(ix, fItems[ix], fItems_aix[ix], sep='_')
            amfNames_ix.append(a)

        # post-processing: there might be ampNames_ix, which are empty. This happens
        # when there is no pItems member with a variable item

        occur_ix=[] # list of indices for occurrences of pathPrefixes
        for jx in range(sz_jx_max):
            occur_ix.append([])
            for ix in range(annot_sz):
                if jx < len(pItems[ix]):
                    occur_ix[jx].append(ix)

        pathPrefix=[]
        for jx in range(sz_jx_max):
            pathPrefix.append([])
            for ix in range(annot_sz):
                if jx < len(pItems[ix]):
                    for i in range(len(pItems[ix][jx])):
                        if pItems[ix][jx][i] != '*':
                            pathPrefix[jx].append(pItems[ix][jx][i])
                    break

        # common path; skip non-project prefix
        # col=0 for pItems[ix][0] == '', when applied from the beginning
        # col=-1 for LOG_PATH_BASE found within pItems

        col=[]
        for jx in range(sz_jx_max):
            if len(pItems[0][jx][0]):
                col.append(-1)
            else:
                col.append(0)

        isBreak=[]

        while True:
            for jx in range(sz_jx_max):
                col[jx] += 1

                if jx == len(isBreak):
                    isBreak.append(False)

                ref_str=''
                for ix in range(annot_sz):
                    if jx < len(pItems[ix]):
                        if len(ref_str) == 0:
                            ref_str = pItems[ix][jx][col[jx]]
                        elif pItems[ix][jx][col[jx]] == '*':
                            isBreak[jx]=True
                            break
                else:
                    if len(ref_str) > 0:
                        for kx in range(annot_sz):
                            if jx < len(pItems[kx]):
                                del pItems[kx][jx][col[jx]]

                col[jx] -= 1  # because the next item slipped into current col

            for jx in range(sz_jx_max):
                if not isBreak[jx]:
                    break
            else:
                break  # this breaks the while loop

        # check whether the first col is always ''
        for jx in range(sz_jx_max):
            isAlways=True
            for ix in range(annot_sz):
                if len(pItems[ix]) > jx:
                    if len(pItems[ix][jx][0]):
                        isAlways=False

            if isAlways:
                for ix in range(annot_sz):
                    if len(pItems[ix]) > jx:
                        del pItems[ix][jx][0]

        # align columns
        ref_val=[]
        for jx in range(sz_jx_max):
            ref_val.append(pItems[sz_jx_max_ix][jx])

        for ix in range(annot_sz):
            for jx in range(sz_jx_max):
                if len(pItems[ix]) > jx:
                    if ref_val[jx] != pItems[ix][jx] and len(pItems[ix][jx]):
                        pItems[ix].insert(jx, [])


        for jx in range(sz_jx_max):
            fO_ix=occur_ix[jx][0]

            name = self.getPathPrefix(pathPrefix[jx])
            fl = os.path.join(self.f_annot, name + '.json')

            with open(fl, 'w') as fd:
                self.write_json_header(pathPrefix[jx], pItems[fO_ix][jx], fd)

                # write annotations for given path prefixes or kind of MIP names
                isInit=True
                #for jx in range(sz_jx_max):
                for ix in occur_ix[jx]:
                    self.write_json_annot(ix, jx,
                            pItems[ix][jx], ampNames_ix[ix][jx],
                            fd, init=isInit )

                    isInit=False

                self.write_json_annot(0, 0,
                    pItems[0][0], ampNames_ix[0][0],
                    fd, final_annot=True, final=True )

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
            items, items_axi = self.getAnnotSource(api)
        else:
            afi = self.annot_fName_id[ix]
            items, items_axi = self.getAnnotSource(afi, sep='_')

        return items, items_axi


    def annotation_amNames(self, ix, xItems, xItems_axi, sep='/'):
        amNames=[]      # name of components in the annotation; list of list
        amNames_ix=[]  # index of var-name in annot_fName_id

        # mutable path/filename items within the current annotation
        for m in range(len(xItems)):
            amNames.append([])
            amNames_ix.append([])
            j=-1

            for n in range(len(xItems[m])):
                if xItems[m][n] == '*':
                    j += 1
                    amNames[m].append([])
                    amNames_ix[m].append({})

                    for i in range(len(xItems_axi[m])):
                        p_id = xItems_axi[m][i]

                        if sep == '/':
                            k = self.path_ids[p_id][n + self.items_del_count[m]]
                            item = self.p_items[k]
                        else:
                            k = self.fName_ids[p_id][n]
                            # find all var_ids containing f_items index jx
                            item = self.f_items[k]

                        try:
                            amNames_ix[m][j][item]
                        except:
                            amNames[m][j].append(item)
                            amNames_ix[m][j][item] = [p_id]
                        else:
                            amNames_ix[m][j][item].append(p_id)


        return amNames_ix


    def annotation_getItemsX(self, ix):
        # ix: index of annotation
        # self.prj_var_ix: path with variable as i-th component
        # self.prj_frq_ix: path with frequency as i-th component

        # number of variables
        sz_var = len(self.annot_fName_id[ix])

        # find identical indices of path and variable items
        # within the annot_..._ids
        api = self.annot_path_id[ix]
        afi = self.annot_fName_id[ix]

        # Find the beginning of a path common for the entire annotation.
        # Note that each index of p_items stores a list of corresponding
        # path components.

        # init of pItems[[]] found for this, i.e. ix-th, annotation.
        # Mutable items are represented by '*'
        # pItems takes into account the possibility that different annotation
        # paths are of different size. pItems_api[[]] gives corresponding
        # api elements.
        pItems, pItems_api = self.getAnnotSource(api)

        if len(pItems[0]) == 0:
            del pItems[0]

        # mutable items within the current annotation
        fItems, fItems_afi = self.getAnnotSource(afi, sep='_')

        # entire experiment, i.e. for all path components?
        if len(afi) == len(self.fName_ids):
            count = 0
            for jx in range(len(self.annot_fName_id[ix])):
                count += len(self.annot_fName_dt_id[ix][jx])

            #if count == self.file_count:
            #    return [['experiment', [path], [] ]]

        ampNames=[]      # name of components in the annotation; list of list
        ampNames_ix=[]  # index of var-name in annot_fName_id

        # mutable path items within the current annotation
        for m in range(len(pItems)):
            ampNames.append([])
            ampNames_ix.append([])
            j=-1

            for n in range(len(pItems[m])):
                if pItems[m][n] == '*':
                    j += 1
                    ampNames[m].append([])
                    ampNames_ix[m].append({})

                    for i in range(len(pItems_api[m])):
                        p_id = pItems_api[m][i]

                        # jx = self.fName_ids[id][self.prj_var_ix]
                        jx = self.path_ids[p_id][n]

                        # find all var_ids containing p_items index jx
                        item = self.p_items[jx]

                        try:
                            ampNames_ix[m][j][item]
                        except:
                            ampNames[m][j].append(item)
                            ampNames_ix[m][j][item] = [p_id]
                        else:
                            ampNames_ix[m][j][item].append(p_id)


        # mutable filename items within the current annotation
        amfNames=[]      # name of components in the annotation; list of list
        amfNames_ix=[]  # index of var-name in annot_fName_id
        amfNames_p_ix=[]
        for m in range(len(fItems)):
            amfNames.append([])
            amfNames_ix.append([])
            amfNames_p_ix.append([])
            j=-1

            for n in range(len(fItems[m])):
                if fItems[m][n] == '*':
                    j += 1
                    amfNames[m].append([])
                    amfNames_ix[m].append({})
                    amfNames_p_ix[m].append({})

                    for i in range(len(fItems_afi[m])):
                        f_id = fItems_afi[m][i]

                        # jx = self.fName_ids[id][self.prj_var_ix]
                        jx = self.fName_ids[f_id][n]

                        # find all var_ids containing p_items index jx
                        item = self.f_items[jx]

                        # find the 'm' index of pItems[m]
                        p_id = self.f_p_ids[f_id]
                        for m2 in range(len(pItems_api)):
                            isBreak=False
                            for n2 in range(len(pItems_api[m2])):
                                if pItems_api[m2][n2] == p_id:
                                    p_id=m2
                                    isBreak
                                    break
                            if isBreak:
                                break

                        try:
                            amfNames_ix[m][j][item]
                        except:
                            amfNames[m][j].append(item)
                            amfNames_ix[m][j][item] = [f_id]
                            amfNames_p_ix[m][j][item] = [p_id]
                        else:
                            amfNames_ix[m][j][item].append(f_id)
                            amfNames_p_ix[m][j][item].append(p_id)


        path=''
        # assemble paths
        scope=['']
        n=0

        '''
        for i in range(1,len(pItems)):
            scope.append([])

            if len(self.p_drs[i]) == 1:
                # static path component
                path += '/' + pItems[i]
                scope[i].append('*')
            elif len(self.p_drs[i]) == len(ampNames[n]):
                # pItems[i] == '*',
                # i.e. all mutable items of the current path component
                path += '/' + pItems[i]
                scope[i].append('*')
                n += 1
            else:
                # a sub-set of all mutable items of the current path component
                scope[i] = ampNames[n]
                n += 1
        '''






        '''
          example:
          ampNames   : [['day', 'mon'],
                        ['tas', 'tasmax', 'tasmin']]
          ampNames_ix: [{'day': [12, 13, 14], 'mon': [34, 35, 36]},
                        {'tasmax': [13, 35], 'tasmin': [14, 36], 'tas': [12, 34]}]

          Note: here, var-acronyms and frequencies appear. The index, which
                corresponds to the variable, has to be determined by comparison
                to self.var_ids, which in turn was derived by use of self.prj_var_ix,
                i.e. the position of the variable in the filename for a given project.
        '''

        # find the ampNames which are associated  e.g. all frequencies
        ampNames_mult=[]
        for i in range(len(ampNames_ix)):
            keys_i = ampNames_ix[i].keys()
            ampNames_mult.append({})

            # The indices pointed to by keys are available in one of the
            # keys of other ampNames_ix keys such that all keys of this
            # particular ampNames_ix item are matched.
            for key_i in keys_i:
                ix_i_vals=ampNames_ix[i][key_i]

                for j in range(len(ampNames_ix)):
                    foundKeys=[]
                    if i == j:  # identity
                        continue

                    keys_j = ampNames_ix[j].keys()
                    for key_j in keys_j:
                        for ix_i_val in ix_i_vals:
                            if ix_i_val in ampNames_ix[j][key_j]:
                                foundKeys.append(key_j)

                    if len(foundKeys) == len(keys_j):
                        ampNames_mult[i][key_i]=foundKeys


        # atomic variable across the entire experiment;
        # note that ampNames are unique

        var_exp=[]
        path_exp=[]
        isAllAtom=[]  # curr annotated var with all frequencies: True
        isFrqAtom=[]  # curr annotated var with less than all frequencies: True

        '''
        for i in range(len(ampNames[vIx]) -1,-1,-1):
            name = ampNames[vIx][i]

            isAllAtom.append([])  # collect freqs for each var
            isFrqAtom.append([])  # collect freqs for each var

            countA=0 # annot_var with available freqs
            countC=0 # corresponding var with all freq

            for f in ampNames_ix[vIx][name]:

                numA = len( self.annot_fName_dt_id[ix][f] )
                countA += numA

                numC = self.var_dt_count[afi[f]]
                countC += numC

                if numA == numC:
                    isFrqAtom[i].append(True)
                else:
                    isFrqAtom[i].append(False)

            if countA == countC:
                isAllAtom.append(True)
            else:
                isAllAtom.append(False)
        '''
        return scope


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
        items_aix=[]

        for i in range(len(a_item_ix)):
            aix=a_item_ix[i]
            itm=[]
            for j in range(len(ref_ids[aix])):
                if not same[j]:
                    itm.append('*')
                else:
                    k = ref_ids[aix][j]
                    itm.append(ref_items[k])

            isNewItm=True
            for j in range(len(items)):
                if itm == items[j]:
                    isNewItm=False
                    items_aix[j].append(aix)
                    break

            if isNewItm:
                items.append(itm)
                items_aix.append([aix])

        if sep == '/':
            if len(self.pathBase):
                for jx in range(len(items)):
                    count=0
                    for i in range(len(items[jx])):
                        if items[jx][i] == self.pathBase:
                            for j in range(i):
                                del items[jx][0]
                                count += 1
                            break

                    try:
                        self.items_del_count
                    except:
                        self.items_del_count = [count]
                    else:
                        self.items_del_count.append(count)
            else:
                try:
                    self.items_del_count
                except:
                    self.items_del_count = [0, 0]

        return items, items_aix


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
                blk.append(line.rstrip(' \n'))

                words = line.split()

                if words[0] == 'PROJECT:' and len(self.project) == 0:
                    self.project = words[1]
                    if words[1] == 'CORDEX':
                        self.isCORDEX = True
                        self.prj_fName_sep = '_'
                        self.prj_path_sep = '/'
                        self.prj_var_ix = 0
                        self.prj_frq_ix = 7

                    elif words[1][0:4] == 'CMIP':
                        self.isCMIP = True
                        self.prj_fName_sep = '_'
                        self.prj_path_sep = '/'
                        self.prj_var_ix = 0
                        self.prj_frq_ix = 1

                elif words[0] == 'LOG_PATH_BASE:':
                        self.pathBase = words[1]

                elif words[0] == 'items:':
                    self.blk_last_line = fd.next().rstrip(' \n')
                    break

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


    def getPathPrefix(self, prefix):
        # default: turn a path into '_' separated string
        # if given, use a project provided identification string instead

        s=''
        for p in prefix:
            if len(p) > 0:
                if len(s) > 0:
                    s += '_'

                s += p

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


    def period_final(self):
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
                if word[1] == frqs[j]:
                    frqs_id[j].append(i)
                    break
            else:
                # append new lists
                frqs.append(word[1])
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
        with open(self.f_perd, 'w') as fd:
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
        f_hr_sum  = os.path.join(self.sum_dir, 'time_range.txt')
        hyphen     = ' - '
        mark_ok    = '    '
        mark_left  = '--> '
        mark_right = ' <--'
        sz_max += 1

        with open(f_hr_sum, 'w') as fd:
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
        self.f_perd  = os.path.join(log_path, 'Period')
        self.sum_dir = os.path.join(log_path, 'Summary', log_name)

        qa_util.mkdirP(self.f_annot)
        qa_util.mkdirP(self.f_perd)
        qa_util.mkdirP(self.sum_dir)

        self.f_perd  = os.path.join(self.f_perd, log_name + '.period')

        # time range of atomic variables; in order to save mem,
        # beg and end, respectively, are linked to the name by the
        # index of the corresponding atomic variable name in var
        self.fName_ids=[]
        self.fName_dt_id={}  # each fName_id gets a list of dt ids
        self.path_ids=[]
        self.f_p_ids={}
        self.f_items=[]
        self.p_items=[]
        self.p_drs=[]

        self.var_ids={}  # contains all [ids] with the same variable name in {}

        self.atomicBeg=[]      # atomic time interval
        self.atomicEnd=[]      # atomic time interval
        self.dt=[]             # time intervals of sub-temp files

        self.annot_capt=[]     # brief annotations
        self.annot_tag=[]      # corresponding impact-tag
        self.annot_scope=[]    # brief annotations
        self.annot_fName_id=[]   # for each var involved
        self.annot_path_id=[]  #
        self.annot_var_ix=[]    # only project variable names
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
                        path_id = self.decomposition(words[1], self.p_items,
                                                     self.path_ids, self.prj_path_sep)

                        # in particular for paths with several variables
                        self.f_p_ids[fName_id_ix] = path_id

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

        # test for ragged time intervals of atomic variables for given frequency
        self.period_final()

        self.annotation_merge()

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


    def write_json_header(self, pathPrefix, pItems, fd):
        # write header of QA results

        if len(pathPrefix):
            if pathPrefix[0] == '':
                del pathPrefix[0]

        while True:
            if len(pItems[0]) == 0:
                del pItems[0]
            else:
                break

        tab='    '

        qa_cnclsn=''

        fd.write('{\n')
        fd.write(tab + '"QA_conclusion": "' + qa_cnclsn + '",\n')
        fd.write(tab + '"project": "' +  self.project + '",\n')

        j=-1

        # remedy when a member of pastPrefix is also in pItems
        raPath=[]
        notAppended=True
        for pP in pathPrefix:
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

        self.shared_DRS=[]
        for i in range(len(raPath)):
            s='"DRS_' + str(i)
            fd.write(tab + s + '": "')

            if raPath[i] == '*':
                self.shared_DRS.append(s)
                fd.write('SHARED",\n')
            else:
                fd.write(raPath[i] + '",\n')

        return


    def write_json_annot(self, ix, jx,
                            pItems, ampNames_ix,
                            fd, init=False, final=False, final_annot=False):
                            #fItems, amfNames_ix,
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

        for k in range(len(ampNames_ix)):
            fd.write(3*tab + self.shared_DRS[k] + '": ')

            keys = str( ampNames_ix[k].keys() )
            keys = keys.replace("'",'"')
            fd.write(keys + ',\n')

        capt = self.annot_capt[ix].strip("'")
        fd.write(3*tab + '"caption": "' + capt + '",\n')

        severe = 'x'
        fd.write(3*tab + '"severity": "' + severe + '"\n')
        fd.write(2*tab + '}')

        return


if __name__ == '__main__':
    summary = LogSummary()
    f_logs = summary.prelude(sys.argv[1:])

    for f_log in f_logs:
        summary.run(f_log) # prelude returns always a list
