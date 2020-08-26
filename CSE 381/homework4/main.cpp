/* 
 * File:   main.cpp
 * Author: Torsten
 * Copyright 2020 overbetn@miamioh.edu
 * Created on February 25, 2020, 4:41 PM
 * 
 * A simple custom shell program that will read user input and then execute that
 * user input to the linux os. Also indicates whether to execute a series of 
 * commands provided in a file in either a serial or parallel fashion.
 */

#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <unordered_map>
#include <iomanip> 
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>

using namespace std;
/*
 * Executes a given set of commands with the execvp call.
 * 
 * @param argList - provided list of arguments that is to be executed.
 */
void myExec(vector<string> argList) {
    std::vector<char*> args;    // list of pointers to args
    for (auto& s : argList) {
        args.push_back(&s[0]);  // address of 1st character
    }
    // nullptr is very important
    args.push_back(nullptr);
    // Make execvp system call to run desired process
    execvp(args[0], &args[0]);
    // In case execvp ever fails, we throw a runtime execption
    throw std::runtime_error("Call to execvp failed!");
}
/*
 * Forks a process and then sends the provided vector holding the list of args
 * to my Exec to then execute those commands. 
 * 
 * @param argList - provided list of arguments that is to be executed.
 */
int forkNexec(const vector<string>& argList) {
    int pid = fork();
    if (pid == 0) {
        myExec(argList);
    }
    return pid;  
}
/*
 * takes in an input stream and a vector and begins to fill that vector with 
 * the contents of that stream.
 * 
 * @param is - an arbitrary input stream that is used to fill a vector
 * 
 * @param args - a given vector of strings that is to be filled with the 
 *               contents of the provided input stream.
 */
void fillVector(istream& is, vector<string>& args) {
    string line;
    while (is >> quoted(line)) {
        args.push_back(line);
    }
}
/*
 * This is a simple method which will print to cout that a given process is 
 * running and then call the forkNexec to begin running that process. 
 * 
 * @param args - this is a string vector that holds the command that the program 
 *               is then supposed to execute.
 * @return int - the program returns the given process id of the process being
 *               executed.
 */
int runProcess(vector<string>& args) {
    cout << "Running:";
    for (size_t i = 0; i < args.size(); i++) {
        cout << " " << args.at(i);
    }
    cout << endl;
    return forkNexec(args);
}
/*
 * Provided piece of code from lab exercise 4 that will simply wait for a 
 * indicated process to finish running and then return its respective exit code.
 * 
 * @param pid - this is simply a given pid that the user would like to wait on
 *              and receive its exit code.
 * @return int - simply returns the exit code of a given process. 
 */
int wait(int pid) {
    int exitCode = 0;
    waitpid(pid, &exitCode, 0);
    return exitCode;
}
/*
 * This method will only ever be called if the user indicated that they are 
 * trying to run a serial or parallel process. Then based on that it will run 
 * the contents of a shell style in either a parallel or serial fashion. 
 * 
 * @param args - this is a string vector which contains the line that was 
 *               indicated to be running either a serial or parallel process
 *               followed by the file to do so.
 */
void runFile(vector<string> args) {
    // This simply opens up the file indicated after the user indicated that 
    // they will be executing a serial or parallel process. Currently it does 
    // not handle the case where a user does not indicate a file name and will
    // throw an error.
    ifstream infile(args.at(1));
    string line;
    // Vector that is used solely for running parallel processes and holds the
    // respective process IDs in order.
    vector<int> pids;
    // integer to hold the exit code of a process 
    int exitCode;
    // Reads the file line by line and parses them as seen fit.
    while (getline(infile, line)) {
        // Checks to make sure the line is not a comment line or that it is not 
        // empty
        if (!line.empty() && line.front() != '#') {
            // Opens string stream from the line read in by the file for easier
            // parsing of that string. 
            istringstream is(line);
            // Creates a vector to be filled with commands provided by the file
            vector<string> vec;
            fillVector(is, vec);
            // If the user indicates to run a parallel process then then pids
            // will begin to be filled with the respective process IDs and 
            // initialize the running of those processes with the runprocess
            // call.
            if (args.at(0) == "PARALLEL") {
                pids.push_back(runProcess(vec));
            // Otherwise the user indicated to run the process in a serial 
            // fashion and the program will begin to exeecute those commands. 
            } else {
                exitCode = wait(runProcess(vec));
                cout << "Exit code: " << exitCode << endl;
            }
        }
    }
    // If the user indicated the running of a parallel process then the program
    // will begin to execute those processes in a parallel fashion with the 
    // help of the pids vector. 
    if (args[0] == "PARALLEL") {
        for (size_t i = 0; i < pids.size(); i++) {
            exitCode = wait(pids.at(i));
            cout << "Exit code: " << exitCode << endl;
        }
    }
    // simply closes the file stream
    infile.close();
}
/*
 * The while loop is constantly searching for user input and then taking that 
 * input line-by-line to then parse it. After parsing those user commands it 
 * will check the beginning of that line to figure out which command the user 
 * is trying to execute.
 * 
 * @return int - should just simply return 0 to indicate successful execution.
 */
int main() {
    string line, cmd;
    int exitCode;
    // Prints a > to indicate the program is waiting for user input and then
    // grabs that line to begin parsing.
    while (cout << "> ", getline(cin, line)) {
        cmd = "";
        // Initialize vector that will store the arguments of the user command
        vector<string> args;
        // Opens string stream to begin parsing the given line.
        istringstream is(line);
        // Grabs the first element of the line to begin searching what the 
        // command that was provided
        is >> cmd;
        // If the command was not empty or a comment it will add that command
        // to the args vector and begin to fill the rest of that vector with the
        // contents of that line.
        if (!(cmd.empty() || cmd == "#")) { 
            args.push_back(cmd);
            fillVector(is, args);
        } else {
            // If the command was empty or a comment it will start the next 
            // iteration of the while loop.
            continue;
        } 
        // If the command input was SERIAL or PARALLEL then the program will be
        // running a file so it will then send the args vector to a separate
        // method to process that file. 
        if (args.at(0) == "SERIAL" || args.at(0) == "PARALLEL") {
            runFile(args);
        // Exit will simply stop the program by breaking the loop
        } else if (args.at(0) == "exit") { 
            break; 
        // If it is any other command the program will simply run the process
        // and obtain that process ID from the wait method call and then 
        // print the respective exit code.
        } else {
            exitCode = wait(runProcess(args));
            cout << "Exit code: " << exitCode << endl;            
        }
    }
    return 0;
}

