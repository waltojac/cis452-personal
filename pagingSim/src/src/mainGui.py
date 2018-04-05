'''
Created on Apr 3, 2018

@author: jacobwalton
'''
from PyQt5 import QtWidgets
import design2, sys
from PyQt5.QtWidgets import QFileDialog, QTableWidgetItem
from Model import fileIn, OperatingSystem, pageTable
import copy
from PyQt5 import QtGui

class App(QtWidgets.QMainWindow, design2.Ui_MainWindow):
    def __init__(self, parent=None):
        super(App, self).__init__(parent)
        self.setupUi(self)
        self.spot = 0
        self.lines = []
        
        #for process display free squares
        self.freeSquares = [0,1,2,3]
        self.freeSquaresHist = []
        self.freeSquaresHist.append(copy.deepcopy(self.freeSquares))

        self.displayed = {}
        self.displayedHist = []
        self.displayedHist.append(copy.deepcopy(self.displayed))

        
        #add action listeners
        self.backPushButton.clicked.connect(self.back)
        self.nextPushButton.clicked.connect(self.next)
        self.actionLoad_Trace.triggered.connect(self.load)
        

    def updateUi(self):    
        #for line number counter    
        self.lineNumber.setText(str(self.spot))
        
        if self.spot < len(self.lines):
            self.lineText.setText(self.lines[self.spot])
        else:
            self.lineText.setText("")

        #colors linked to pids
        colors = {0: "grey", 1: "orange", 2: "teal", 3: "yellow", 4:"green", 5: "purple", 6: "red", 7:"pink"}
        
        #for the main memory table, display all frames
        for i, frame in enumerate(self.OS.pageTables):
            #for frames that are in use, -1 will not be the page #
            if frame.pNum != -1:
                item1 = QtWidgets.QTableWidgetItem(str(frame.fNum))
                item1.setBackground(QtGui.QColor(colors[frame.pid]))
                self.tableWidget.setItem(i,0, item1)
                
                item2 = QtWidgets.QTableWidgetItem(str(frame.pid))
                item2.setBackground(QtGui.QColor(colors[frame.pid]))
                self.tableWidget.setItem(i,1, item2)
                
                item3 = QtWidgets.QTableWidgetItem(str(frame.segment))
                item3.setBackground(QtGui.QColor(colors[frame.pid]))
                self.tableWidget.setItem(i,2, item3)
                
                item4 = QtWidgets.QTableWidgetItem(str(frame.pNum))
                item4.setBackground(QtGui.QColor(colors[frame.pid]))
                self.tableWidget.setItem(i,3, item4)
            
            #when the frame is empty
            else:
                self.tableWidget.setItem(i,0, QTableWidgetItem(str(frame.fNum)))
                self.tableWidget.setItem(i,1, QTableWidgetItem(str(frame.pid)))
                self.tableWidget.setItem(i,2, QTableWidgetItem(str(frame.segment)))
                self.tableWidget.setItem(i,3, QTableWidgetItem(""))
                
        
        
        #sort by page number so that it displays correctly 
        self.OS.pageTables.sort(key=lambda x: x.pNum)
        
        #for the process squares
        for proc in self.OS.processes:
            output = ""
            output += "--- Process " + str(proc) + " ---\n"
            output += "          Page    Frame\n"
            text = "Text"
            data = "Data    "
            for frame in self.OS.pageTables:
                if frame.pid == proc:
                    if frame.segment == "Text":
                        text += "\t" + str(frame.pNum) + "       " + str(frame.fNum) + "\n"
                    if frame.segment == "Data":
                        data += "\t" + str(frame.pNum) + "       " + str(frame.fNum) + "\n"
            output += text + data
            
            
            self.freeSquares.sort()
            
            color = ""
            color = "background-color: " + colors[proc]
            
            if int(proc) not in self.displayed:
                selection = self.freeSquares.pop(0)
                self.displayed.__setitem__(proc, selection)
                
                if (selection == 0):
                    self.processTable0.setText(output)
                    self.processTable0.setStyleSheet(color)
                elif (selection == 1):
                    self.processTable1.setText(output)
                    self.processTable1.setStyleSheet(color)

                elif (selection == 2):
                    self.processTable2.setText(output)
                    self.processTable2.setStyleSheet(color)

                elif (selection == 3):
                    self.processTable3.setText(output)
                    self.processTable3.setStyleSheet(color)

            else:
                selection = self.displayed[proc]
                if (selection == 0):
                    self.processTable0.setText(output)
                    self.processTable0.setStyleSheet(color)
                elif (selection == 1):
                    self.processTable1.setText(output)
                    self.processTable1.setStyleSheet(color)
                elif (selection == 2):
                    self.processTable2.setText(output)
                    self.processTable2.setStyleSheet(color)
                elif (selection == 3):
                    self.processTable3.setText(output)
                    self.processTable3.setStyleSheet(color)

        #sort back to frame number
        self.OS.pageTables.sort(key=lambda x: x.fNum)
                
        
    def load(self):
        #set the defaults
        self.lines = []
        self.OS = OperatingSystem()
        self.state = []
        self.cp = copy.deepcopy(self.OS)
        self.state.append(self.cp)
        self.spot = 0
        self.end = True
        
        
        file = self.openFileNameDialog()
        self.traceName.setText(file)
        self.lines = fileIn(file)
        self.end = False
        
        self.updateUi()
        
    def openFileNameDialog(self):    
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        fileName, _ = QFileDialog.getOpenFileName(self,"QFileDialog.getOpenFileName()", "","All Files (*);;Python Files (*.py)", options=options)
        if fileName:
            return fileName
    
    def next(self):
        if (self.spot + 1) > self.lines.__len__():
            self.end = True
        else:
            self.end = False
        if (self.end == False):
            line = ""
            line = self.lines[self.spot]
            self.spot += 1
                
            #add the process to M
            if ("Halt" not in line):
                pageTable(line, self.OS)
                self.updateUi()

            #remove the process from M
            else:
                #get the individual words from the line
                word = []
                word = line.split()
                self.OS.removeProc(int(word[0]))
                
                #clear the process table
                selection = self.displayed[int(word[0])]
                self.freeSquares.append(selection)

                #link the selection to the label and clear the label
                if (selection == 0):
                    self.processTable0.setText("")
                    self.processTable0.setStyleSheet("")
                elif (selection == 1):
                    self.processTable1.setText("")
                    self.processTable1.setStyleSheet("")
                elif (selection == 2):
                    self.processTable2.setText("")
                    self.processTable2.setStyleSheet("")
                elif (selection == 3):
                    self.processTable3.setText("")
                    self.processTable3.setStyleSheet("")

                #remove the process from the displayed dict
                del self.displayed[int(word[0])]
                self.updateUi()
                
            #save a copy of the current state for a back button
            cp = copy.deepcopy(self.OS)
            self.state.append(cp)    
            self.displayedHist.append(copy.deepcopy(self.displayed))
            self.freeSquaresHist.append(copy.deepcopy(self.freeSquares))
            


    
    def back(self):
        #clear all initially
        self.processTable0.setText("")
        self.processTable1.setText("")
        self.processTable2.setText("")
        self.processTable3.setText("")
        self.processTable0.setStyleSheet("")
        self.processTable1.setStyleSheet("")
        self.processTable2.setStyleSheet("")
        self.processTable3.setStyleSheet("")
        
        if (self.spot - 1) < 0:
            self.end = True
        else:
            self.end = False
            
        if (self.end == False):
            self.state.pop(self.spot)
            self.displayedHist.pop(self.spot)
            self.freeSquaresHist.pop(self.spot)
            self.spot -= 1
            
            #restore the OS to the previous state
            self.OS = copy.deepcopy(self.state[self.spot])
            self.displayed = copy.deepcopy(self.displayedHist[self.spot])
            self.freeSquares = copy.deepcopy(self.freeSquaresHist[self.spot])
            
            
            self.updateUi()
            
        




        
def main():
    app = QtWidgets.QApplication(sys.argv)
    form = App()
    form.show()
    app.exec_()
    
    

if __name__ == '__main__':
    main()
    
