/* Copyright (C) overbetn@miamioh.edu
 * File:   homework5.cpp
 *
 */

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include "Movie.h"

// A couple of namespaces to streamline code
using namespace std;
using namespace boost::asio::ip;

// Alias to a vector-of-strings
using StrVec = std::vector<std::string>;

/** Helper method to split a given string into separate words based on spaces.
 * Note that this method does the following 1 extra operations:
 *    2. Removes all commas (',') characters
 */
StrVec split(std::string str) {
    // Change all ',' to spaces
    std::replace(str.begin(), str.end(), ',', ' ');
    // Now use a istringstream to extract words into a vector
    std::istringstream is(str);
    StrVec wordList;
    std::string word;
    while (is >> std::quoted(word)) {
        wordList.push_back(word);
    }
    // Return the list of words back to the caller.
    return wordList;
}
// Breaks up the given string to analyze what is needed
// determines if there is a "where" to see if it needs to process a like 
// request
bool where(StrVec &words, bool &special) {
    for (size_t i = 0; i < words.size(); i++) {
        if (words.at(i) == "where") {
            special = true;
            return true;
        }
    }
    return false;
}
// find the position of "like"
int findLike(StrVec &words) {
    int pos = 0;
    for (int i = 0; words[i] != "like"; i++) {
        pos = i;
    }
    return pos + 1;
}
int findFrom(StrVec &words) {
    int pos = 0;
    for (size_t i = 0; i < words.size(); i++) {
        if (words.at(i) == "from") {
            return i;
        }
        // pos = i;
    }
    return pos + 1;
}
void analyze(StrVec &words, std::string &url, std::string &bLike,
        std::string &aLike, bool &special) {    
    StrVec bFrom;
    int pos = 0;
    int fr = findFrom(words);
    // Loop to iterate and find words in between select and from
    for (int i = 1; i < fr; i++) {
        bFrom.push_back(words[i]);
        pos = i;
    }
    // Saves the position of "like"
    if (where(words, special)) {
        int likePos = findLike(words);
        bLike = words.at(likePos - 1);
        aLike = words.at(likePos + 1);
        std::quoted(aLike);
    }
    url = words[fr];
    words = bFrom;
    std::quoted(url);
}
// Trims the url to be rea dable for the is
void trimURL(std::string &url, std::string &path) {
    url = url.substr(7);
    int pos = url.find('/');
    path = url.substr(pos);
    url = url.substr(0, pos);
}
// Prints the movie info out to cout
void print(StrVec &words, Movie m, std::ostream& os, bool &special,
        std::string bLike, std::string aLike) {
    if (special == false) {
        for (size_t i = 0; i < words.size(); i++) {
            if (words.at(i) == "title") {
                os << quoted(m.getCol(words.at(i))) << " ";
            } else {
                os << m.getCol(words.at(i)) << " ";
            }
        }
        os << std::endl;
    } else {
        for (size_t i = 0; i < words.size(); i++) {
            if (m.getCol(bLike).find(aLike) != string::npos) {
                if (words.at(i) == "title") {
                    os << quoted(m.getCol(words.at(i))) << " ";
                } else {
                    os << m.getCol(words.at(i)) << " ";
                }
                os << std::endl;
            }
            // os << std::endl;
        }        
    }
}
void process(std::istream& is, std::ostream& os, std::string &host,
        StrVec &words, std::string &path, bool &special, std::string &aLike,
        std::string &bLike) {
    // Send HTTP GET request to server
    os << "GET " + path + " HTTP/1.1\r\n";  
    os << "Host: " << host << "\r\n";  
    os << "Connection: Close\r\n\r\n";
    // Read line-by-line of response and print it.
    std::string line;
    Movie m;
    while (line != "\r") {
        std::getline(is, line);
    }
    std::getline(is, line);
    while (std::getline(is, line)) {
        std::istringstream iss(line);
        iss >> m;
        print(words, m, std::cout, special, aLike, bLike);
    }
}
int main() {
    std::string input, host, path, bLike, aLike;
    bool special = false;
    // Keeps getting user input until exit command
    while (std::getline(std::cin, input)) {
        std::cout << "query> ";
        if (input == "exit")
            return 0;
        std::cout << input;
        StrVec words = split(input);
        
        analyze(words, host, aLike, bLike, special);
        
        trimURL(host, path);
        tcp::iostream stream(host, "80");
        process(stream, stream, host, words, path, special, bLike, aLike);
    }
    return 0;
}
