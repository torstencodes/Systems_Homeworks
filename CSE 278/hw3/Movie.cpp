/*
 * Copyright (C) 2019 overbetn@miamiOH.edu
 */
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include "Movie.h"

Movie::Movie(int movieID, const std::string& title, int year, 
            const std::string genres, int imdbId, float rating,
            int numRaters) : movieID(movieID), title(title), year(year),
        genres(genres), imdbId(imdbId), rating(rating), 
        numRaters(numRaters) {    
}

Movie::~Movie() {    
}

std::ostream& operator<<(std::ostream& os, const Movie& mov) {
    // Implement this method to print id, name, and email for per.
    // For name and email used std::quoted to automatically add quotes
    // about them.
    os << mov.movieID << " " << std::quoted(mov.title) << " " 
       << mov.year << " " << std::quoted(mov.genres) << " "
       << mov.imdbId << " " << mov.rating << " " << mov.numRaters;
    // Never forget to return the stream back.
    return os;
}

std::istream& operator>>(std::istream& is, Movie& mov) {
    // Implement this method to read id, name, and email for per.
    // For name and email used std::quoted to automatically handle quotes
    // in inputs
    is >> mov.movieID >> std::quoted(mov.title) >> mov.year >> 
       std::quoted(mov.genres) >> mov.imdbId >> mov.rating >> mov.numRaters;
    // Never forget to return the stream back.
    return is;
}

std::string to_string(const Movie& mov) {
    std::ostringstream os;
    os << mov;
    return os.str();
}
