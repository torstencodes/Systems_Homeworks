// Copyright (C) overbetn@miamioh.edu
#define MYSQLPP_MYSQL_HEADERS_BURIED
#include <mysql++/mysql++.h>
#include <string>
#include <iostream>
#include "Movie.h"

/** A fixed HTML header that is printed at the beginning of output to ensure
 * the output is displayed correct.
 */
const std::string HTMLHeader = 
    "Content-Type: text/html\r\n\r\n"
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "<link type='text/css' rel='stylesheet' href='movie.css'/>\n"
    "</head>\n"
    "<body>";

/** A fixed HTML footer that is printed at the end of output to ensure
 * correct HTML formatting
 */
const std::string HTMLFooter = "</body>\n</html>";
// writes and prints the matching movie 
void print(const mysqlpp::StoreQueryResult& result, Movie& m) {
    std::cout << HTMLHeader << std::endl;
    for (const auto& row : result) {
        m.movieID = row["id"];
        m.title = row["title"].c_str();
        m.genres = row["genres"].c_str();
        m.imdbId = row["imdb_id"];
        m.rating = row["rating"];
        m.numRaters = row["raters"];
        m.year = row["year"];
        m.printAsHtml(std::cout);
    }
    std::cout << HTMLFooter << std::endl;
}
/** Convenience method to decode HTML/URL encoded strings.

    This method must be used to decode query string parameters
    supplied along with GET request.  This method converts URL encoded
    entities in the from %nn (where 'n' is a hexadecimal digit) to
    corresponding ASCII characters.

    \param[in] str The string to be decoded.  If the string does not
    have any URL encoded characters then this original string is
    returned.  So it is always safe to call this method!

    \return The decoded string.
*/
// Takes a CGI input and then decodes into strings
std::string url_decode(std::string str) {
    // Decode entities in the from "%xx"
    size_t pos = 0;
    while ((pos = str.find_first_of("%+", pos)) != std::string::npos) {
        switch (str.at(pos)) {
            case '+': str.replace(pos, 1, " ");
            break;
            case '%': {
                std::string hex = str.substr(pos + 1, 2);
                char ascii = std::stoi(hex, nullptr, 16);
                str.replace(pos, 3, 1, ascii);
            }
        }
        pos++;
    }
    return str;
}
// Grabs the cgi user input and assigns it to related variables
void getVals(auto& para1, auto& val1, auto& para2, auto& val2, auto& para3,
             auto& val3, auto& para4, auto& val4) {
    // std::string input = "title=&genre=C%2B%2B&startYear=&endYear=";
    // std::istringstream is(input);
    std::getline(std::cin, para1, '=');
    std::getline(std::cin, val1, '&');
    std::getline(std::cin, para2, '=');
    std::getline(std::cin, val2, '&');
    std::getline(std::cin, para3, '=');
    std::getline(std::cin, val3, '&');
    std::getline(std::cin, para4, '=');
    std::getline(std::cin, val4, '&');
}
// Checks if there is a title and genre specified and will then add to the 
// query string. 
void makeQuery(std::string& qString, std::string genre, std::string title) {
    if (title != "")
        qString += " AND title LIKE '%%%0%%'";
    if (genre != "") 
        qString += " AND genres LIKE '%%%1%%'";
}
int main() {
    std::string para1, para2, val1, val2, para3, val3, para4, val4, qString;
    int startY = 1;
    int endY = 2100;
    getVals(para1, val1, para2, val2, para3, val3, para4, val4);
    const std::string title = url_decode(val1);
    const std::string genre = url_decode(val2);
    std::string startY1 = url_decode(val3).substr(0, 4);
    std::string endY1 = url_decode(val4).substr(0, 4);
    // Checks if there were specified years
    if (startY1 != "" && startY1 != "\n") 
        startY = std::stoi(startY1);
    if (endY1 != "" && endY1 != "\n") 
        endY = std::stoi(endY1);
    mysqlpp::Connection myDB("cse278s19", "os1.csi.miamioh.edu", "cse278s19",
            "rbHkqL64VpcJ2ezj");
    // Create a query
    mysqlpp::Query query = myDB.query();
    // Creates a string to be edited and then sent to the query 
    qString = "SELECT * FROM Movies WHERE year >= %2 AND year <= %3";
    // calls the method to  edit the query string
    makeQuery(qString, genre, title);
    query << qString << ";";
    query.parse();  // check to ensure query is correct.
    // Sets bind variables for the query
    mysqlpp::StoreQueryResult result = query.store(title, genre, startY, endY);
    Movie m;
    // Prints resulted movies
    print(result, m);
    return 0;
}

