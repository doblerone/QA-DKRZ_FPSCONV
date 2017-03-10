import sys

blk=[]
ix=-1
blkOn=False
lines=[]
for line in sys.stdin:
    lines.append(line)

#with open( "/hdh/hdh/QA-DKRZ/test/cmorOut.txt") as fd:
#    for line in fd:
#        lines.append(line)

for line in lines:
    # find the beginning of a block
    count = line.count('!')

    if count > 5:
        if blkOn:
            blkOn=False
        else:
            blkOn=True
            blk.append([])
            ix += 1

        continue

    if blkOn:
        blk[ix].append(line)


# remove disturbing sub-strings
for ix in range(len(blk)):
    for jx in range(len(blk[ix])-1,-1,-1):
        # rm line-feed
        blk[ix][jx] = blk[ix][jx].strip("\t\n !")
        #blk[ix][jx].strip()
        if len(blk[ix][jx]) == 0:
            del  blk[ix][jx]
        else:
            # replace every ';' by '.', because semi-colon serves as line delimiter
            # in qa-exec-check
            if blk[ix][jx].find(';'):
                blk[ix][jx] = blk[ix][jx].replace(';', '.')

# re-sort according to annots (terminated py a period)
annot=[]

# remove the 'Ceterum censeo Karthaginem esse delendam' phrase
cicero=blk[len(blk)-1]
if cicero[0].find('The input file is not CMIP6 compliant'):
    del blk[len(blk)-1]
del cicero

for ix in range(len(blk)):
    annot.append([])
    phrase=''

    for line in blk[ix]:
        last=len(line)-1
        pos0=0

        while True:
            if pos0 == 0 and len(phrase) > 0:
                phrase += ' '

            pos = line.find('.', pos0)
            if pos == -1:
                phrase += line[pos0:]
                break
            else:
                # there is at least one period somewhere in the line
                if last == pos:
                    # current line ends with a period
                    annot[ix].append(phrase + line[pos0:pos+1])
                    phrase=''
                    break
                elif line[pos+1] == ' ':
                    # current line contains a period terminating the current annot
                    annot[ix].append(phrase + line[pos0:pos+1])
                    phrase=''
                    pos += 1
                else:
                    phrase += line[pos0:pos+1]

                pos0 = pos+1

        #if len(phrase):
        #    annot[ix].append(phrase)
        #    phrase=''

    if len(phrase):
        annot[ix].append(phrase)

for ix in range(len(annot)):
    # the first annot of each annotatiopn group is a caption
    # the tag equals the length of the caption
    flag = ' FLAG-BEGL1-CMOR_' + str(len(annot[ix][0])) + ':CAPT-BEG'
    flag += annot[ix][0] + 'CAPT-END'

    annot[ix][0] = flag

    sz = len(annot[ix])

    if sz > 1:
        annot[ix][0] += 'TXT-BEG'
        for jx in range(1,sz):
            if jx > 1:
                annot[ix][0] += ';'

            annot[ix][0] += annot[ix][jx]

        annot[ix][0] += 'TXT-END'

        for jx in range(1,sz):
            del annot[ix][jx]

    annot[ix][0] += 'FLAG-END'
    annot[ix][0] = annot[ix][0].replace('"','\"')


for ix in range(len(annot)):
  print annot[ix][0],
