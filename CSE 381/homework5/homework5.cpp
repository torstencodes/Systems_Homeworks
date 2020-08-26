/* 
 * A simple web-server.  
 * 
 * The web-server performs the following tasks:
 * 
 *     1. Accepts connection from a client.
 *     2. Processes cgi-bin GET request.
 *     3. If not cgi-bin, it responds with the specific file or a 404.
 * 
 * Copyright (C) overbetn@miamioh.edu
 */

#include <ext/stdio_filebuf.h>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

// Using namespaces to streamline code below
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

/** Forward declaration for method defined further below.  You will
    need to use this url_decode method in your serveClient method.
 */
std::string url_decode(std::string data);

// Named-constants to keep pipe code readable below
const int READ = 0, WRITE = 1;

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
 * Forks a process and then sends the provided vector holding the list of args
 * to my Exec to then execute those commands. The parent process does all of the
 * printing after the first few lines of the header. 
 * 
 * @param argList - provided list of arguments that is to be executed.
 * 
 * @param os - This is simply the output stream that the server will be writing
 *             to.
 * 
 * @return int - returns the pid of the child process being executed.
 */
int forkNexec(const vector<string>& argList, std::ostream& os) {
    int pipefd[2];
    pipe(pipefd); 
    const int pid = fork();
    if (pid == 0) {
        close(pipefd[READ]); 
        dup2(pipefd[WRITE], WRITE);
        myExec(argList);
    } else {
        close(pipefd[WRITE]);
        __gnu_cxx::stdio_filebuf<char> fb(pipefd[READ], std::ios::in);
        std::istream is(&fb);
        for (std::string line; std::getline(is, line);) {
            os << std::hex << line.size() + 1 << "\r\n" << line << "\r\n\r\n";
        }  // Process child data (send as chunked HTTP response)
        const int exitCode = wait(pid);
        const std::string exitStr = "Exit code: " + std::to_string(exitCode);
        os << std::hex << exitStr.length() << "\r\n" << exitStr << "\r\n"
           << "0" << "\r\n" << "\r\n";
    }
    waitpid(pid, nullptr, 0);
    return pid;
}
/*
 * When the url being sent does not contain a GET request to the path
 * "/cig-bin/exec" then the program will call this method to send a basic 
 * error response.
 * 
 * @param str - This is the url string the was sent to the server and is parsed
 *              in this method to grab what is needed for the error response.
 * 
 * @param os - This is simply the output stream that the server will be writing
 *             to 
 */
void errorResponse(const std::string& str, std::ostream& os) {
    std::string str1 = str.substr(str.find(" "), str.rfind(" ") - 3);
    str1 = str1.substr(2);
    const std::string invalid = "Invalid request: " + str1;
    os << "HTTP/1.1 404 Not Found\r\n" << "Content-Type: text/plain\r\n" 
       << "Transfer-Encoding: chunked\r\n" << "Connection: Close\r\n"
       << "\r\n" << std::hex << invalid.length() + 1 << "\r\n" << invalid
       << "\r\n\r\n0\r\n" << std::endl;
}
/*
 * This method simply takes a string that contains the URL after it has been 
 * decoded and then parses it with the help of string streams and getlines. 
 * The parsing is simply to grab the necessary commands and arguments. After
 * that it then converts that string to a vector so it can be used in the 
 * execvp call.
 * 
 * @param url - This is the url that is being sent through the GET request 
 *              to the server 
 * 
 * @return std::vector<string> - simply returns a vector that contains the args
 *                               to be executed by the server.
 */
std::vector<string> parseUrl(const std::string& url) {
    std::string dummy, cmds;
    std::vector<string> cmdsVec;
    std::istringstream is(url);
    std::getline(is, dummy, '?');
    std::getline(is, dummy, '=');
    std::getline(is, dummy, '&');
    cmds += dummy;
    std::getline(is, dummy, '=');
    while (std::getline(is, dummy, ' ') && (dummy.substr(0, 4) != "HTTP")) {
        cmds += " " + dummy;
    }
    std::istringstream in(cmds);
    while (in >> quoted(dummy)) {
        cmdsVec.push_back(dummy);
    }
    return cmdsVec;
}
/**
 * Process HTTP request (from first line & headers) and
 * provide suitable HTTP response back to the client.
 * 
 * @param is The input stream to read data from client.
 * 
 * @param os The output stream to send data to client.
 */
void serveClient(std::istream& is, std::ostream& os) {
    // Implement this method as per homework requirement. Obviously
    // you should be thinking of structuring your program using helper
    // methods first. You should add comemnts to your helper methods
    // and then go about implementing them.
    std::string url = "";
    std::getline(is, url);
    url = url_decode(url);
    if (url.substr(0, 17) != "GET /cgi-bin/exec") {
        errorResponse(url, os);
    } else {
        os << "HTTP/1.1 200 OK\r\n" << "Content-Type: text/plain\r\n"
           << "Transfer-Encoding: chunked\r\n" << "Connection: Close\r\n\r\n"; 
        std::vector<std::string> cmds = parseUrl(url);
        int childPid = forkNexec(cmds, os);
        wait(childPid);
    }
}

// -----------------------------------------------------------
//       DO  NOT  ADD  OR MODIFY CODE BELOW THIS LINE
// -----------------------------------------------------------

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

/**
 * Runs the program as a server that listens to incoming connections.
 * 
 * @param port The port number on which the server should listen.
 */
void runServer(int port) {
    io_service service;
    // Create end point
    tcp::endpoint myEndpoint(tcp::v4(), port);
    // Create a socket that accepts connections
    tcp::acceptor server(service, myEndpoint);
    std::cout << "Server is listening on port "
              << server.local_endpoint().port() << std::endl;
    // Process client connections one-by-one...forever
    while (true) {
        tcp::iostream client;
        // Wait for a client to connect
        server.accept(*client.rdbuf());
        // Process information from client.
        serveClient(client, client);
    }
}

/*
 * The main method that performs the basic task of accepting connections
 * from the user.
 */
int main(int argc, char** argv) {
    if (argc == 2) {
        // Process 1 request from specified file for functional testing
        std::ifstream input(argv[1]);
        serveClient(input, std::cout);
    } else {
        // Run the server on some available port number.
        runServer(0);
    }
    return 0;
}

// End of source code
