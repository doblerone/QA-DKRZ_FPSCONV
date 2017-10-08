'''
Created on 21.03.2016

@author: hdh
'''

import qa_util
from qa_exec import QaExec

class QaLauncher(object):
    '''
    classdocs
    '''
    task_finished = False

    def __init__(self, log, qaConf, g_vars):
        # connection of any executing entity, here for QA-DKRZ
        self.qa_exec = QaExec(log, qaConf, g_vars)

        self.g_vars = g_vars

        self.is_next=False
        if qaConf.isOpt('NEXT'):
            self.is_next=True
            self.next=qaConf.getOpt('NEXT')


    def run(self, data_path, fName, t_vars):
        # print fName
        return self.qa_exec.run(t_vars)


    def start(self, queue):

        while True:
            if QaLauncher.task_finished:
                break

            try:
                (data_path, fNames, t_vars) = queue.get()
            except:
                pass
            else:
                if data_path == '---EOQ---':
                    queue.task_done()
                    QaLauncher.task_finished=True
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

                    if not self.run(data_path, fNames[ix], t_vars):
                        break

                # for the previous range == 1
                if self.is_next and self.next == 0:
                    return False

            queue.task_done()


        return
