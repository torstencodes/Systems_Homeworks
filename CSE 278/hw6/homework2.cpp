/* 
 * File:   homework2.cpp
 * Author: Torsten Overbeck
 * Copyright (C) 2019 overbetn@miamioh.edu
 * Created on February 11, 2019, 8:27 PM
 */

#include <iostream>
#include <string>
#include <sstream>
#include "Dictionary.h"

using namespace std;

/** A simple string to print the HTML header. */
const std::string HTML_HEADER =
        "<html>\n"
        "  <body>\n"
        "    <table border=\"1\">\n"
        "      <tr><td>First and last line</td><td>Word counts</td></tr>\n";

/** A simple string to finish the HTML. */
const std::string HTML_TRAILER =
        "    </table>\n"
        "  </body>\n"
        "</html>\n";

/**
 * Convenience method to count number words and number of valid English
 * words in a given line.
 *
 * @param dict The dictionary to be used to clean-up words and check for
 * valid English words.
 * 
 * @param line The line whose words are to be counted by this method.
 *
 * @param wordCounts The current set of counters to be updated by this
 *     method.  This array should have 2 entries, first one being number
 *     of words and the second number is number of valid English words.
 * 
 */
void updateWordCounts(const Dictionary& dict, const std::string& line, 
        int wordCounts[]) {
    // Create a istringstream to read words from the given line.
    std::istringstream wordin(line);
    // Now you can use wordin the same way as std::cin
    // Process each word and update counters
    string word;
    while (wordin >> word) {
        wordCounts[0]++;
        word = dict.toEngWord(word);
        if (dict.isEnglishWord(word)) {
            wordCounts[1]++;
        }
    }
}

/** 
 * Helper method to print summary information about a paragraph in HTML
 * format. 
 * 
 * @param firstLine The first line of the paragraph to be printed.
 * @param lastLine The last line of the paragraph to be printed.
 * @param wordCount An array with count of words and count of valid
 *     English words.
 */
void printInfo(const std::string& firstLine, const std::string& lastLine,
        int wordCount[]) {
    // Print the HTML out in the necessary format
    if (firstLine != lastLine) {
        std::cout << "      <tr><td>" << firstLine << "<br>" << lastLine <<
                "</td><td>Words: " << wordCount[0] << "<br>English words: " <<
                wordCount[1] << "</td></tr>\n";
    } else {
        std::cout << "      <tr><td>" << firstLine << "<br></td><td>Words: " << 
                wordCount[0] << "<br>English words: " << wordCount[1] << 
                "</td></tr>\n";
    }
    // Reset the word counts
    wordCount[0] = wordCount[1] = 0;
}

/*
 * The main method that reads & processes line-by-line from std::cin
 */
int main(int argc, char** argv) {
    // Create the dictionary object to be used in this program.
    Dictionary dict;
    // Write rest of the logic here to process inputs from std::cin.
    std::cout << HTML_HEADER;
    std::string prevLine = "", firstLine = "", currLine = "";
    int wordCounter[2] = {0, 0};
    while (std::getline(std::cin, currLine)) {
        if (prevLine == "" && currLine != "") {
            firstLine = currLine;
        }
        updateWordCounts(dict, currLine, wordCounter);
        if (currLine == "" && prevLine != "") {
            printInfo(firstLine, prevLine, wordCounter);
        }
        prevLine = currLine;
    }
    if (prevLine != "") {
        printInfo(firstLine, prevLine, wordCounter);
    }
    std::cout << HTML_TRAILER;
    // All done.
    return 0;
} 
