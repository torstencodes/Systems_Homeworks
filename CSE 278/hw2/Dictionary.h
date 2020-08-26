/* 
 * File:   Dictionary.h
 * Author: raodm
 *
 * Copyright (C) 2019 raodm@miamiOH.edu
 */

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cctype>

/** A simple class to operate as an English dictionary. Use this class as:
 * 
 * #include "Dictionary.h"
 * 
 * int main() {
 *     Dictionary dict;
 *     std::string word = dict.toEngWord("te,st;");
 *     bool isValid = dict.isValidWord(word);
 * }
 */
class Dictionary {
public:
    /** The default (and only) constructor
     * 
     * @param[in] engFilePath Path to the file with valid English words.
     */
    Dictionary(const std::string& engFilePath = "english.txt") {
        // Read valid English words from a file.
        std::ifstream dict(engFilePath);
        // Use a convenient stream-iterator to read words until EOF
        std::istream_iterator<std::string> words(dict), eof;
        // Store list of valid words in our instance variable
        validWords = std::vector<std::string>(words, eof);
    }

    /** Main method to determine if a given word is a valid English word.
     * 
     * @param[in] word The word to be checked to see if it is a valid 
     * English word. Note that use the toEngWord to clean-up the word first.
     * 
     * @return This method returns true if the word is a valid English word.
     * Otherwise this method returns false.
     */
    bool isEnglishWord(const std::string& word) const {
        return std::binary_search(validWords.begin(), validWords.end(),
                word);
    }
    
    /** Helper method to remove punctuation marks in words and convert
     * it to lowercase.
     * 
     * @param[in] word The string to be cleaned-up by removing punctuation
     * marks (if any) from this string.
     * 
     * @return This method returns the cleaned-up word.
     */
    std::string toEngWord(std::string word) const {
        // Remove all punctuations from the word.
        word.erase(std::remove_if(word.begin(), word.end(), ispunct), 
                word.end());
        // Convert all letters to lower case
        std::transform(word.begin(), word.end(), word.begin(), tolower);
        // Return the cleaned-up word.
        return word;
    }

private:
    /** The list of valid English words loaded from the dictionary file.
     * This list is set in the constructor and is never changed during the
     * lifetime of this class.
     */
    std::vector<std::string> validWords;
};


#endif /* DICTIONARY_H */

