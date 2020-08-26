/* Copyright (C) overbetn@miamioh.edu
 * File:   Server.cpp
 * 
 */

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include "Server.h"

// The default file to return for "/"
const std::string RootFile = "index.html";

const std::string msg = "<html>\n"
    "<body>\n"
    "<h1>Testing</h1>\n"
    "<p>A simple para</p>\n"
    "</body>\n"
    "</html>\n";

Server::Server() {
    // Nothing to be done in the constructor (for now).
}

Server::~Server() {
    // Nothing to be done in the destructor.
}

// Checks if the file is valid and returns resulting 404 or 200 codes
bool isFGood(std::string filePath) {
    std::ifstream data(filePath);
    if (!data.good()) {
        return false;
    } return true;
}
// Takes the filePath as a param and then returns the byte size of the file
int getFileSize(std::string filePath) {
    std::ifstream data(filePath);
    data.seekg(0, std::ios::end);
    int fileSize = data.tellg();
    data.seekg(0);
    return fileSize;
}
// Checks if for the file type and returns a string that says what kind of file
// was entered.
std::string fileType(std::string filePath) {
    std::string type = "text/plain";
    if (filePath.find(".html") != std::string::npos) {
        type = "text/html";
    } else if (filePath.find(".png") != std::string::npos) {
        type = "image/png";
    } else if (filePath.find(".jpg") != std::string::npos) {
        type = "image/jpeg";
    } else {
        type = "text/plain";
    }
    return type;
}
// Prints the content of the file passed into the method and then sends it to
// the output stream
void processFile(std::string filePath, std::ostream& os) {
    std::ifstream data(filePath);
    std::string input;
    while (std::getline(data, input)) {
        os << input << std::endl;
    }
}
// If the file was not found, this method returns the correct 404 headers
void get404(std::string filePath, std::ostream& os) {
    std::string msg404 = "The following file was not found: " + filePath + 
                         "\r\n";
    os << "HTTP/1.1 404 Not Found \r\n"
       << "Server: SimpleServer\r\n"
       << "Content-Length: " << msg404.size() - 1 << "\r\n"
       << "Connection: Close\r\n"
       << "Content-Type: text/plain\r\n\r\n"
       << msg404;
}
// Returns the correct header if the file was found
void getHeader(std::string filePath, std::ostream& os) {
    os << "HTTP/1.1 200 OK\r\n"
       << "Server: SimpleServer\r\n"
       << "Content-Length: " << getFileSize(filePath) << "\r\n"
       << "Connection: Close\r\n"
       << "Content-Type: " + fileType(filePath) + "\r\n\r\n";
    processFile(filePath, os);
}

void Server::serveClient(std::istream& is, std::ostream& os) {
    // First extract request line from client
    // The line is in the form GET Path_to_file HTTP/1.1
    std::string line;
    std::getline(is, line);
    
    // Now modify the code to extract the 2nd word from line to
    // determine the file name! Use line.find(' ', indexPos) method to
    // extract the Path_to_file and print it.
    const size_t spc1Pos = line.find(' ');
    const size_t spc2Pos = line.find(' ', spc1Pos + 1);
    std::string filePath = line.substr(spc1Pos + 1,
          spc2Pos - spc1Pos - 1);
    if (filePath == "/") {
        filePath = RootFile;
    } else {
        filePath = line.substr(spc1Pos + 2, spc2Pos - spc1Pos - 2);
    }
    // Send a fixed HTTP response back to the client.
    
    // Checks if the file path is good and returns the right header
    if (isFGood(filePath)) {
        getHeader(filePath, os);
    } else {
        get404(filePath, os);
    }
}

void Server::runServer() {
    using namespace boost::asio;
    using namespace boost::asio::ip;
    io_service service;
    // Create end point. 0 == Pick any free port.
    tcp::endpoint myEndpoint(tcp::v4(), 0);
    // Create a socket that accepts connections
    tcp::acceptor server(service, myEndpoint);
    std::cout << "Listening on port " << server.local_endpoint().port()
              << std::endl;
    // Process client connections one-by-one...forever
    while (true) {
        tcp::iostream client;
        // Wait for a client to connect
        server.accept(*client.rdbuf());
        // Process information from client.
        serveClient(client, client);
    }
}
