/* 
 * File:   main.cpp
 * Author: Torsten
 * Copyright 2020 overbetn@miamioh.edu
 * 
 * This is a simple program that traverses a given PID in a given file. The file
 * contains processes from a Linux operating system. This program simply follows
 * those processes down to root and prints those respective processes. 
 * 
 * Created on February 16, 2020, 10:38 AM
 */

#ifndef MAIN_CPP
#define MAIN_CPP

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <iomanip> 
#include <sstream>
#include <algorithm>
#include <vector>
#include "overbetn_hw3.h"

using namespace std;
// No argument constructor for class implementation
processTree::processTree() {}
// No argument destructor for class implementation
processTree::~processTree() {}
/*
 * This method utilizes stream extraction and grabs all the respective data
 * required for each unordered_map. There is an unordered_map that maps the PID
 * to its PPID and a map that maps the PID to the command executed with respect
 * to that process ID. 
 * 
 * @param inFile - the input stream for the desired file to be read.
 */
void processTree::loadData(ifstream &inFile) {
    string uid, c, stime, tty, time, cmd, temp;
    int pid, ppid;
    getline(inFile, temp);
    while (getline(inFile, temp)) {
        istringstream is(temp);
        is >> uid >> pid >> ppid >> c >> stime >> tty >> time;
        getline(is, cmd);
        PtoPP[pid] = ppid;
        PtoCMD[pid] = cmd;
    }
}
/*
 * This method simply takes the desired process to traverse and then sends that
 * PID to a sorting method that fills up the vector<int> instance variable. That
 * instance variable keeps track of the order in which the processes are traced
 * from their PPIDs. Using that vector, this method grabs each key in the vector
 * in its respective order and prints until the vector is empty.
 * 
 * @param input - the desired key to follow
 */
void processTree::printProcess(int input) {
    int currentKey;
    sort(input);
    cout << "Process tree for PID: " << input << "\nPID\tPPID\tCMD" << endl;
    while (pidsInOrder.size() >= 1) {
        currentKey = pidsInOrder.back();
        cout << currentKey << "\t" << PtoPP[currentKey] << "\t" << 
                PtoCMD[currentKey] << endl;
        pidsInOrder.pop_back();
    }    
}
/*
 * This method simply pairs each PID to its respective PPID to traverse the
 * processes back to root. It keeps track of the process order by pushing the 
 * respective process IDs into a vector that stores the order in which the 
 * processes are executed. Once the process ID is equal to 1, we know that the 
 * sort successfully made it to root and stops the recursion there. 
 * 
 * @param input - this is the current PID that is going to matched to a PPID
 * 
 */   
void processTree::sort(int input) {
    auto key = PtoPP.find(input);
    if (input == 1) {
        pidsInOrder.push_back(input);
    } else {
        auto key2 = PtoPP.find(key->second);
        pidsInOrder.push_back(input);
        sort(key2->first);
    }    
}

#endif
/*
 * The main method starts by creating a processTree object, this is a class that 
 * solely traverses a process by its parent processes back to its root.
 * Afterwards, it then creates a file stream and integer based on the user input
 * which is then sent to the object to processed and then print the respective
 * data.
 * 
 * @param argc - This is the count of user provided arguments, in this case it 
 * is not so important since the test cases provided will only contain two args.
 * This could be useful when the user provides too little arguments to run the 
 * program. 
 * 
 * @param argv - An array of the user provided command-line arguments. The first
 * argument is the specified file that is being read from. The second is the PID
 * tree that the program will be tracing.
 * 
 */
int main(int argc, char** argv) {
    processTree process;
    ifstream inFile(argv[1]);
    int in = stoi(argv[2]);
    if (!inFile.good()) {
        cout << "Bad file!" << endl;
        return 0;
    }
    process.loadData(inFile);
    process.printProcess(in);
    return 0;
}

