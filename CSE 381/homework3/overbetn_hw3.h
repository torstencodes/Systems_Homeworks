/* 
 * File:   overbetn_hw3.h
 * Author: Torsten
 * Copyright 2020 overbetn@miamioh.edu
 * 
 * Created on February 16, 2020, 10:48 AM
 */
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <iomanip> 
#include <sstream>
#include <algorithm>
#include <vector>

#ifndef OVERBETN_HW3_H
#define OVERBETN_HW3_H
/*
 * Class declaration for a simple class that traverses through PIDs and PPIDs 
 * all the way back to the root. 
 */
class processTree {
public:
    // No argument constructor declared for the class.
    processTree(); 
    /*
     * Loads the data from the provided file stream into the respective 
     * unordered_maps, one that maps PIDs to PPIDs, and another map that has 
     * PIDs mapped to the command executed by that process.
     * 
     * @param inFile - the input file stream provided by the user
     */
    void loadData(std::ifstream& inFile);
    /*
     * This uses the sorted vector of PIDs returned by the private sort method, 
     * and the unordered_maps that were filled in loadData, to print the 
     * proper tree of processes. It does so by grabbing each key form the vector
     * and inserting those keys into the maps to grab the proper data to print.
     * 
     * Sample output:
     * Process tree for PID: 26863
     *   PID     PPID    CMD
     *   1       0        /sbin/init
     *   949     1        /usr/sbin/sshd -D
     *   19083   949      sshd: yangz12 [priv]
     * 
     * @param input - the provided PID to traverse which is provided by the user
     */
    void printProcess(int input);
    // No argument destructor declared for this class. 
    ~processTree();
    
private:
    // vector of PIDs that store the PIDs going from the top level ID in the 
    // first position to root in the last. 
    std::vector<int> pidsInOrder;
    // unordered_map that maps a given PID to its respective PPID
    std::unordered_map<int, int> PtoPP;
    // unordered_map that maps a PID to its command executed
    std::unordered_map<int, std::string> PtoCMD;
    /*
     * Fills a vector that holds each PID in the order that they are executed. 
     * First position being the highest level PID with the last position storing
     * the root.
     * 
     * @param input - the provided PID to traverse, which is provided by the 
     * printprocess method. This is where sort is called.  
     */
    void sort(int input);
};

#endif /* OVERBETN_HW3_H */
