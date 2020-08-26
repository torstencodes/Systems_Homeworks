/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   homework3.cpp
 * Author: Torsten Overbeck
 * Copyright (C) 2019 overbetn@miamiOH.edu
 * Created on February 18, 2019, 10:21 AM
 */

#include <unordered_map>
#include <fstream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include "Movie.h"

using namespace std;

using MovieMap = std::unordered_map<int, Movie>;
/*
 * 
 */

MovieMap load(const std::string& filePath) {
    // Create the entry for the file to be read
    MovieMap db;
    std::ifstream data(filePath);
    if (!data.good()) {
        throw std::runtime_error("Unable to read file " + filePath);
    }
    std::string stupid;
    std::getline(data, stupid);
    // Create the map to be populated and returned

    // Load person information into the DB by reading 
    // entry-by-entry and then add them to the unordered_map using
    // operator[].  Use the ID of the person as the key.
    Movie mov;  // temporary object
    while (data >> mov) {
        // Implement rest of the method here.
        int id = mov.getmovieID();
        db[id] = mov;
    }
    // Return the map of people back
    return db;
}

void findMovieID(const MovieMap& db, istringstream& str) {
    int id;
    str >> id;
    // Use the find method to find entry and print it. The find method
    // is covered in lecture slides!
    if (db.find(id) != db.end()) {
        std::cout << db.at(id) << std::endl;
    } else {
        std::cout << "Move with ID  " << id << " not found in database." 
                  << std::endl;
    }
}

void searchMovie(const MovieMap& db, istringstream& str) {
    std::string searchStr;  // Information to search for
    str >> std::quoted(searchStr);
    int count = 0;
    
    // For each entry convert the value (i.e., second) for each element 
    // using the to_string method and then use std::string::find method 
    // to see if searchStr is a substring. If so, print the value.
    for (const auto& entry : db) {
        const std::string info = to_string(entry.second);
        if (info.find(searchStr) != std::string::npos) {
            std::cout << entry.second << std::endl;
            count++;
        }
    }
    std::cout << "Found " << count << " matche(s)." << endl;
}

int main(int argc, char** argv) {
    MovieMap db = load("movies_db.txt");
    string input;
    do {
        std::cout << "Enter a command:" << std::endl;
        getline(std::cin, input);
        istringstream in(input);
        in >> input;
        if (input == "search") {
            searchMovie(db, in);
        } else if (input == "find") {
            findMovieID(db, in);
        }
    } while (input != "exit");
    return 0;
}
