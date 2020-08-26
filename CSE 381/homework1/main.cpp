/**
 * File: exercise1.cpp
 * Author: Torsten Overbeck
 * 
 * Copyright (C) 2020 overbetn@miamioh.edu
 * 
 * A program that takes user input from the command line to then read employee
 * sales data from a give .tsv file. In the command line the user will specify 
 * which file to read from, which employee to return sales data about and then
 * whether or not to return the max employee information.
 */ 
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <iomanip> 
#include <sstream>
#include <algorithm>
#include <utility>
/*
 Structure that stores the employee name and their sales number. This was used
 * to store the data for the employee with the maximum sales.
 */
struct maxEmployee {
    int max;
    std::string employeeName;
}; 
/**
 This method finds the employee with the maximum number of sales and adds them 
 * to a structure that stores that maximum employee data.
 * 
 * @param employeeN is a reference to the employee information that is requested
 * by the user. 
 */
maxEmployee findMax(const std::unordered_map<std::string, int> &employeeMap) {
    maxEmployee currentMax;
    currentMax.max = 0;
    for (std::pair<std::string, int> pairs : employeeMap) {
        if (pairs.second > currentMax.max) {
            currentMax.employeeName = pairs.first;
            currentMax.max = pairs.second;
        }
    }
    return currentMax;
}
/*
 This method will print the employee data that is requested by the user 
 * depending on the different arguments provided by the user.
 * 
 * @param employeeMap is a reference to the unordered map that contains all of
 * the employee data given in the file that was read earlier.
 * 
 * @param getMax is a boolean value that determines whether or not to print the
 * maximum employee data
 * 
 * @param employeeN is a reference to the employee information that is requested
 * by the user.
 */
void printEmployee(const std::unordered_map<std::string, int> &employeeMap,
                   const bool getMax, const std::string &employeeN) {
    auto key = employeeMap.find(employeeN);
    if (key == employeeMap.end()) {
        std::cout << "Employee " << employeeN << " not found." << std::endl;
    } else if (getMax) {
        auto currentMax = findMax(employeeMap);
        std::cout << "Sales by " << key->first << " = " << key->second 
                  << std::endl;
        std::cout << "Top employee: " << currentMax.employeeName 
                  << " with sales: " << currentMax.max << std::endl;
    } else {
        std::cout << "Sales by " << key->first << " = " << key->second 
                  << std::endl;
    }    
}
/*
 This method takes in the file name from the command line args and then scans
 * the employee data from that file. It does so by opening a file stream and
 * checking if it is a valid file path. If it is, it will then skip over the 
 * headers with the first while loop and then start fetching the employee data
 * and storing it in an unordered map. The unordered map uses the employee name 
 * as the key and the value is the total sales made by that employee.
 * 
 * @param fileN this is a reference passed by that supplies the file path
 * 
 * @param employeeMap this is a reference to the unordered map that stores the 
 * employee's data
 */
void scanData(const std::string& fileN, 
              std::unordered_map<std::string, int> &employeeMap) {
    std::ifstream in(fileN);
    std::string empName, hold;
    int sales;
    if (!in.good()) {
        std::cout << "The file is not valid!" << std::endl;
    } else {
        while (std::getline(in, hold) && !hold.empty() && (hold != "\r")) {}
        std::getline(in, hold);
        while (in >> std::quoted(empName) >> std::quoted(hold) >> hold) {
            sales = std::stoi(hold);
            employeeMap[empName] += sales;
        }
    }
} 
/*
 This method takes in a string and returns a boolean value of true or false
 * depending on whether the string is "true" or "false"
 * 
 * @param max is the 3rd input from the command line args that determines 
 * whether to return the employee with the maximum sales 
 */
bool getMaxBool(const std::string& max) {
    if (max == "true")
        return true;
    else 
        return false;
}
/*
 The main method simply takes in command line arguments and stores them 
 * respectively into strings that hold the file name, employee name, and a 
 * string that represents a boolean. That last string is converted to
 * a proper boolean by being sent to the getMaxBool method and it represents
 * whether the employee with the maximum number of sales should be included in
 * the output. The unordered_map storing employee data is declared here which is
 * referenced in the rest of the program. Lastly, all the values from the 
 * command line are then sent to their respective methods to execute the 
 * program. 
 * 
 * @param argc is the count of arguments sent throughout the command line
 * 
 * @param argv[] is an array that stores the different arguments passed in the 
 * command line. 
 */
int main(int argc, char *argv[]) {
    std::string fileN = argv[1];
    std::string employeeN = argv[2];
    std::quoted(employeeN);
    bool getMax = getMaxBool(argv[3]);   
    std::unordered_map<std::string, int> employeeMap;
    scanData(fileN, employeeMap);
    printEmployee(employeeMap, getMax, employeeN);
    return 0;
}
