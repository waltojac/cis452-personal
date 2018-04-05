'''
Title: OS Paging Simulator
Date: Mar 26, 2018
Description: Simulates a small paging system on an OS. 

@author: Jacob Walton
'''

import copy
from math import ceil

#class used to represent a frame in Physical M
class Frame():
    def __init__(self, frame, p, seg, page):
        self.fNum = frame
        self.pid = p
        self.segment = seg
        self.pNum = page
        
#class to represent a simple OS
class OperatingSystem():
    
    def __init__(self):
        #free page list
        self.freePages = [0,1,2,3,4,5,6,7]
        #memory page table
        self.pageTables = []
        #process list
        self.processes = []
        
        #add empty frames to all spots in M
        for i in range(8):
            #-1 so that all pNums can be integer type for sort
            f = Frame(i," "," ", -1)
            self.pageTables.append(f)
        
    #function to display the M page table and each process page table
    def printTable(self):
        print("Frame #   Pid   \tSegment    Page #") 
        print("---------------------------------------")
        for frame in self.pageTables:
            #so that all pNums can be int
            if frame.pNum != -1:
                print(" " + str(frame.fNum) +"\t   " + str(frame.pid) + "\t  " + str(frame.segment) + "  \t     " + str(frame.pNum))
            else:
                print(" " + str(frame.fNum) +"\t   " + str(frame.pid) + "\t  " + str(frame.segment) + "  \t       " )

         
         
         
        #sort by page number so that it displays correctly 
        self.pageTables.sort(key=lambda x: x.pNum)
        
        #display the process page tables
        print
        print("--------------------")
        print ("Page Tables:")
        for proc in self.processes:
            print("--- Process " + str(proc) + " ---")
            print ("\tPage    Frame")
            text = "Text"
            data = "Data    "
            for frame in self.pageTables:
                if frame.pid == proc:
                    if frame.segment == "Text":
                        text += "\t" + str(frame.pNum) + "       " + str(frame.fNum) + "\n"
                    if frame.segment == "Data":
                        data += "\t" + str(frame.pNum) + "       " + str(frame.fNum) + "\n"
            print (text + data)
            
        #sort back to frame number
        self.pageTables.sort(key=lambda x: x.fNum)

            
    #function called when Halt is entered
    def removeProc(self, pid):
        #remove from process list
        self.processes.remove(pid)
        self.processes.sort()
        
        #remove all frames associated with the process
        for i in range(len(self.pageTables)):
            if (self.pageTables[i].pid == pid):
                #create new frame to add
                fNum = self.pageTables[i].fNum
                self.freePages.append(fNum)
                f = Frame(fNum," "," ", -1)
                
                #remove the frame
                self.pageTables.remove(self.pageTables[i])
                
                #add the new empty frame to M
                self.pageTables.append(f)
                                
                #sort the pageTable by frame number
                self.pageTables.sort(key=lambda x: x.fNum)
        
                
    
#class to represent a process page table   
class pageTable():
    
    def __init__(self, line, OS):
        words = line.split()
        self.pid = words[0] 
        OS.processes.append(int(self.pid)) 
        OS.processes.sort()
        
        #dictionary for page table since only one key/value for each frame slot
        self.textTable = {}
        self.dataTable = {}
        
        #find number of each type of page
        textPages = int(ceil(int(words[1])/512.0))
        dataPages = int(ceil(int(words[2])/512.0))
        
        #create Text page table
        for i in range(textPages):
            val = OS.freePages.pop(0)
            
            #remove empty frame
            for frame in OS.pageTables:
                if (frame.fNum == val):
                    OS.pageTables.remove(frame)
            
            #create new frame
            f = Frame(int(val), int(self.pid), "Text", i)
            OS.pageTables.append(f)
            self.textTable[i] = val
            
        #create Data page table
        for i in range(dataPages):
            val = OS.freePages.pop(0)
            
            #remove empty frame
            for frame in OS.pageTables:
                if (frame.fNum == val):
                    OS.pageTables.remove(frame)
            
            #create new frame
            f = Frame(int(val), int(self.pid), "Data", i)
            OS.pageTables.append(f)
            self.dataTable[i] = val

        #sort the pageTable by frame number
        OS.pageTables.sort(key=lambda x: x.fNum)
    

def main ():
    fileName = str(input("Enter trace file name: "))
    lines = fileIn(fileName)
    OS = OperatingSystem()
    state = []
    cp = copy.deepcopy(OS)
    state.append(cp)
    spot = 0

    
    val = input("Enter n for next, b for back, or q for quit: ")
    while (val == "n" or val == "b"):
        #end the trace 
        if (spot == len(lines)):
            print("Trace is over, Shutting down...")
            val == "exit"
            break
        
        #go to next line in trace
        if (val == "n"):
            
            line = lines[spot]
            spot += 1

            print (line)
            
            #add the process to M
            if ("Halt" not in line):
                pageTable(line, OS)
                OS.printTable()
            
            #remove the process from M
            else:
                word = line.split()
                print(word[0])
                OS.removeProc(int(word[0]))
                OS.printTable()
                
            #save a copy of the current state for a back button
            cp = copy.deepcopy(OS)
            state.append(cp)
            val = input("Enter n for next trace, or anything else for quit: ")
        
        #go back a step
        if (val == "b"):
            state.pop(spot)
            spot -= 1
            
            #restore the OS to the previous state
            OS = copy.deepcopy(state[spot])
            
            OS.printTable()
            val = input("Enter n for next trace, or anything else for quit: ")
    
    
#function to read the trace file into a string
def fileIn(fileName):
    #grab the input from the file
    file = open(fileName).read().splitlines()
    return file

