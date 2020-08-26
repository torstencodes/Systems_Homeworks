/*
 * Copyright (C) 2019 overbetn@miamiOH.edu
 */

/* 
 * File:   Movie.h
 * Author: tover
 * 
 * Created on February 18, 2019, 10:23 AM
 */

#ifndef MOVIE_H
#define MOVIE_H

#include <iostream>
#include <string>

class Movie {
    // Stream insertion operator to write/print Person object
    friend std::ostream& operator<<(std::ostream& os, const Movie& mov);
    // Stream extraction operator to read/populate a Person object.
    friend std::istream& operator>>(std::istream& is, Movie& mov);
    
private:
    // Instance variables  
    int movieID;
    std::string title;
    int year; 
    std::string genres;
    int imdbId;
    float rating;
    int numRaters;

public:
    int x;
    // Movie constructor
    Movie(int movieID = -1, const std::string& title = "", int year = -1, 
            const std::string genres = "", int imdbId = -1, float rating = -1,
            int numRaters = -1);
    // Movie destructor
    virtual ~Movie();
    // Getter method(s)
    int getmovieID() const { return movieID; }
    int getYear() const { return year; }
    int getimdbID() const { return imdbId; }
    float getRating() const { return rating; }
    int getRaters() const { return numRaters; }    
};

std::string to_string(const Movie& mov);

#endif /* MOVIE_H */

