/* 
 * A simple web-server.  
 * 
 * The web-server performs the following tasks:
 * 
 *     1. Accepts connection from a client.
 *     2. Processes cgi-bin GET request.
 *     3. If not cgi-bin, it responds with the specific file or a 404.
 *     4. Returns stats about the command executed on the server.
 * 
 * Copyright (C) 2020 overbetn@miamioh.edu
 */

#include <ext/stdio_filebuf.h>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/asio.hpp>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <thread>

// Using namespaces to streamline code below
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;



// Forward declaration for method defined further below
void serveClient(std::istream& is, std::ostream& os, bool genFlag);

// shared_ptr is a garbage collected pointer!
using TcpStreamPtr = std::shared_ptr<tcp::iostream>;

/** Simple method to be run from a separate thread.
 *
 * @param client The client socket to be processed.
 */
void threadMain(TcpStreamPtr client) {
    // Call routine/regular helper method.
    serveClient(*client, *client, true);
}

/**
 * Runs the program as a server that listens to incoming connections.
 * 
 * @param port The port number on which the server should listen.
 */
void runServer(int port) {
    // Setup a server socket to accept connections on the socket
    io_service service;
    // Create end point
    tcp::endpoint myEndpoint(tcp::v4(), port);
    // Create a socket that accepts connections
    tcp::acceptor server(service, myEndpoint);
    std::cout << "Server is listening on "
              << server.local_endpoint().port()
              << " & ready to process clients...\n";
    // Process client connections one-by-one...forever
    while (true) {
        // Create garbage-collect object on heap
        TcpStreamPtr client = std::make_shared<tcp::iostream>();
        // Wait for a client to connect
        server.accept(*client->rdbuf());
        // Create a separate thread to process the client.
        std::thread thr(threadMain, client);
        thr.detach();
    }
}

// Forward declaration for method used further below.
std::string url_decode(std::string);

// Named-constants to keep pipe code readable below
const int READ = 0, WRITE = 1;

// The default file to return for "/"
const std::string RootFile = "index.html";

// The following are constant strings that contain the required html
// to generate the appropriate html 

const std::string stats1 = "     </textarea>\n"
     "     <h2>Runtime statistics</h2>\n"
     "     <table>\n"
     "       <tr><th>Time (sec)</th><th>User time</th><th>System time</th><th>"
     "Memory (MB)</th></tr>\n";

const std::string chartString = "     </table>\n"
    "     <div id='chart' style='width: 900px; height: 500px'></div>\n"
    "  </body>\n"
    "  <script type='text/javascript'>\n"
    "    function getChartData() {\n"
    "      return google.visualization.arrayToDataTable(\n"
    "        [\n"
    "          ['Time (sec)', 'CPU Usage', 'Memory Usage'],\n";

const std::string noChart = "     </table>\n"
    "     <div id='chart' style='width: 900px; height: 500px'></div>\n"
    "  </body>\n"
    "  <script type='text/javascript'>\n"
    "    function getChartData() {\n"
    "      return google.visualization.arrayToDataTable(\n"
    "        [\n"
    "          ['Time (sec)', 'CPU Usage', 'Memory Usage']\n";

const std::string htmlEnd = "        ]\n"
    "      );\n"
    "    }\n"
    "  </script>\n"
    "</html>\n";

const std::string htmlResp = "<html>\n"
    "  <head>\n"
    "    <script type='text/javascript' src='"
    "https://www.gstatic.com/charts/loader.js'></script>\n"
    "    <script type='text/javascript' src='/draw_chart.js'></script>\n"
    "    <link rel='stylesheet' type='text/css' href='/mystyle.css'>\n"
    "  </head>\n\n"
    "  <body>\n"
    "    <h3>Output from program</h3>\n"
    "    <textarea style='width: 700px; height: 200px'>\n";

/**
 * This method is a convenience method that extracts file or command
 * from a string of the form: "GET <path> HTTP/1.1"
 * 
 * @param req The request from which the file path is to be extracted.
 * @return The path to the file requested
 */
std::string getFilePath(const std::string& req) {
    // std::cout << req << std::endl;
    size_t spc1 = req.find(' '), spc2 = req.rfind(' ');
    if ((spc1 == std::string::npos) || (spc2 == std::string::npos)) {
        return "";  // Invalid or unhandled type of request.
    }
    std::string path = req.substr(spc1 + 2, spc2 - spc1 - 2);
    if (path == "") {
        return RootFile;  // default root file
    }
    return path;
}

/** Helper method to send HTTP 404 message back to the client.

    This method is called in cases where the specified file name is
    invalid.  This method uses the specified path and sends a suitable
    404 response back to client.

    \param[out] os The output stream to where the data is to be
    written.

    \param[in] path The file path that is invalid.
 */
void send404(std::ostream& os, const std::string& path) {
    const std::string msg = "The following file was not found: " + path;
    // Send a fixed message back to the client.
    os << "HTTP/1.1 404 Not Found\r\n"
       << "Content-Type: text/plain\r\n"
       << "Transfer-Encoding: chunked\r\n"        
       << "Connection: Close\r\n\r\n";
    // Send the chunked data to client.
    os << std::hex << msg.size() << "\r\n";
    // Write the actual data for the line.
    os << msg << "\r\n";
    // Send trailer out to end stream to client.
    os << "0\r\n\r\n";
}

/**
 * Obtain the mime type of data based on file extension.
 * 
 * @param path The path from where the file extension is to be determined.
 * 
 * @return The mime type associated with the contents of the file.
 */
std::string getMimeType(const std::string& path) {
    const size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        const std::string ext = path.substr(dotPos + 1);
        if (ext == "html") {
            return "text/html";
        } else if (ext == "png") {
            return "image/png";
        } else if (ext == "jpg") {
            return "image/jpeg";
        }
    }
    // In all cases return default mime type.
    return "text/plain";
}

/** Convenience method to split a given string into words.

    This method is just a copy-paste of example code from lecture
    slides.

    \param[in] str The string to be split

    \return A vector of words
 */
std::vector<std::string> split(std::string str) {
    std::istringstream is(str);
    std::string word;
    std::vector<std::string> list;
    while (is >> word) {
        list.push_back(word);
    }
    return list;
}

/** Uses execvp to run the child process.

    This method is a helper method that is used to run the child
    process using the execvp command.  This method is called onlyl on
    the child process from the exec() method in this source file.

    \param[in] argList The list of command-line arguments.  The 1st
    entry is assumed to the command to be executed.
*/
void runChild(std::vector<std::string> argList) {
    // Setup the command-line arguments for execvp.  The following
    // code is a copy-paste from lecture slides.
    std::vector<char*> args;
    for (size_t i = 0; (i < argList.size()); i++) {
        args.push_back(&argList[i][0]);
    }
    // nullptr is very important
    args.push_back(nullptr);
    // Finally run the command in child process
    execvp(args[0], &args[0]);  // Run the command!
    // If control drops here, then the command was not found!
    std::cout << "Command " << argList[0] << " not found!\n";
    // Exit out of child process with error exit code.    
    exit(0);
}
/*
 * Helper method that grabs that takes a pid, opens its /proc/pid/stat file,
 * creates a stream and then grabs the stats from that stream. It takes the
 * stats from that stream and then returns a vector of those stats. I believe an
 * unordered map would've been a better data structure to use here although.
 * 
 * @param pid - the pid which we should be retrieving stats about.
 * 
 * @return - returns vector of the stats recorded at that instance.
 */
std::vector<std::string> getStats(const int pid) {
    std::ifstream is("/proc/" + std::to_string(pid) + "/stat");
    std::vector<std::string> vec;
    string dummy;
    if (is.good()) {
        for (int i = 0; (is >> dummy); i++) {
            if (i == 13) {
                dummy = std::to_string(static_cast<int>(std::round(
                        std::stof(dummy) / sysconf(_SC_CLK_TCK))));
                vec.push_back(dummy);
            } else if (i == 14) {
                dummy = std::to_string(static_cast<int>(std::round(
                        std::stof(dummy) / sysconf(_SC_CLK_TCK))));
                vec.push_back(dummy);
            } else if (i == 22) {
                dummy = std::to_string(static_cast<int>(std::stol(dummy) / 
                        std::pow(10, 6)));
                vec.push_back(dummy);
            }
        }
    }
    return vec;
}
/*
 * Takes a vector and updates it with the stats recorded from the getStats 
 * method every second while the process is running. Once the process is
 * done running it will update leave the statsToPrint vector completely 
 * updated with the stats from the give pid.
 * 
 * @param pid - pid to record stats from
 * 
 * @param statsToPrint - reference to vector that is storing all the stats from
 *                       the given pid. 
 */
void getPidInfo(const int pid, std::vector<string> &statsToPrint) {
    if (pid != -1) {
        int exitCode = 0;
        std::vector<std::string> statsVec;
        size_t sleepCount = 1;
        while (waitpid(pid, &exitCode, WNOHANG) == 0) {
            sleep(1);  // sleep for 1 second.
            // Record current statistics about the process.
            statsVec = getStats(pid);
            statsToPrint.push_back(std::to_string(sleepCount));
            statsToPrint.push_back(statsVec.at(0));
            statsToPrint.push_back(statsVec.at(1));
            statsToPrint.push_back(statsVec.at(2));
            sleepCount++;        
        }
    }
}
/*
 * Simple helper method that will add the stats from the base case to the JS 
 * to be printed.
 * 
 * @param vec - vector containing the stats for the given pid
 * 
 * @param response - the response string to be sent 
 */
void printChart(const std::vector<string>& vec, std::string& response) {
    size_t size = vec.size();
    int dummy;
    // response += ",\n";
    for (size_t i = 0; i < size - 4; i = i + 4) {
        response += "          [" + vec[i] + ",";
        dummy = std::stoi(vec[i + 1]) + std::stoi(vec[i + 2]);
        response += std::to_string(dummy) + ",";
        response += vec[i + 3] + "],\n";
    }
    dummy = std::stoi(vec[size - 3]) + std::stoi(vec[size - 2]);
    response += "          [" + vec[size - 4] + "," + std::to_string(dummy) 
             + "," + vec[size - 1] + "]\n";
}
/*
 * Takes a vector that contains the given stats for the current running process
 * and then adds the appropiate table entries to print out. If genChart is true
 * then a separate method will be called to add the chart stats.
 * 
 * @param vec - vector that contains all of the stats data to be printed
 * 
 * @param os - The output stream to print back to the client
 * 
 * @param genChart - boolean that tells the program to generate the chart if 
 *                   true and not to otherwise.
 */
void sendStats(const std::vector<string>& vec, std::ostream& os, 
        const bool genChart) {
    std::string response = stats1;
    int i = 0;
    for (auto x : vec) {
        if (i == 0) {
            response += "       <tr><td>" + x + "</td>";
            i++;
        } else if (i == 3) {
            response += "<td>" + x + "</td></tr>\n";
            i = 0;
        } else {
            response += "<td>" + x + "</td>";
            i++;
        }
    }
    if (genChart) {
        response += chartString;
        printChart(vec, response);
    } else {
        response += noChart;
    }
    response += htmlEnd;
    os << std::hex << response.size() << "\r\n" << response << "\r\n";    
}
/*
 * Helper method to send the correct HTTP response header
 * 
 * @param os - The output stream to print back to the client
 * 
 * @param mimeType - String that contains the mime type
 */
void printHeader(std::ostream& os, const std::string& mimeType) {
    // First write the fixed HTTP header.
    os << "HTTP/1.1 200 OK\r\n"
       << "Content-Type: " << mimeType << "\r\n"        
       << "Transfer-Encoding: chunked\r\n"
       << "Connection: Close\r\n\r\n";
}

/** Helper method to send the data to client in chunks.
    
    This method is a helper method that is used to send data to the
    client line-by-line.

    \param[in] mimeType The Mime Type to be included in the header.

    \param[in] pid An optional PID for the child process.  If it is
    -1, it is ignored.  Otherwise it is used to determine the exit
    code of the child process and send it back to the client.
    
 *  \param[in] is - input stream from child process that will provide the 
 *                  output from that child process.
 * 
 *  \param[out] os - output stream that will send data to the client 
 * 
 *  \param[in] genChart - boolean that will determine whether or not to output
 *                        a chart. 
*/
void sendData(const std::string& mimeType, int pid, std::istream& is, 
        std::ostream& os, const bool genChart) {
    printHeader(os, mimeType);
    std::vector<string> statsToPrint;
    std::thread thr(getPidInfo, pid, std::ref(statsToPrint));
    if (pid != -1) {          
        os << std::hex << htmlResp.size() << "\r\n" << htmlResp << "\n";
    }
    // Read line-by line from child-process and write results to
    // client.
    std::string line;
    while (std::getline(is, line)) {
        // Add required "\n" to terminate the line.
        line += "\n";
        // Add size of line in hex
        os << std::hex << line.size() << "\r\n" << line << "\r\n";
    }
    thr.join();
    // Check if we need to end out exit code
    if (pid != -1) {
        // Wait for process to finish and get exit code.
        int exitCode = 0;
        waitpid(pid, &exitCode, 0);
        // Create exit code information and send to client.
        line = "\r\nExit code: " + std::to_string(exitCode) + "\r\n";
        os << std::hex << line.size() << "\r\n" << line << "\r\n";
        sendStats(statsToPrint, os, genChart);
    }
    // Send trailer out to end stream to client. 
    os << "0\r\n\r\n";
}

/** Run the specified command and send output back to the user.

    This method runs the specified command and sends the data back to
    the client using chunked-style response.

    \param[in] cmd The command to be executed

    \param[in] args The command-line arguments, wich each one
    separated by one or more blank spaces.

    \param[out] os The output stream to which outputs from child
    process are to be sent.
 * 
 *  \param[in] genChart - boolean that will determine whether or not to output
 *                        a chart. 
*/
void exec(std::string cmd, std::string args, std::ostream& os,
        const bool genChart) {
    // Split string into individual command-line arguments.
    std::vector<std::string> cmdArgs = split(args);
    // Add command as the first of cmdArgs as per convention.
    cmdArgs.insert(cmdArgs.begin(), cmd);
    // Setup pipes to obtain inputs from child process
    int pipefd[2];
    pipe(pipefd);
    // Finally fork and exec with parent having more work to do.
    const int pid = fork();
    if (pid == 0) {
        close(pipefd[READ]);        // Close unused end.
        dup2(pipefd[WRITE], 1);     // Tie/redirect std::cout of command
        runChild(cmdArgs);
        // thr.join();
    } else {
        // In parent process. First close unused end of the pipe and
        // read standard inputs.
        close(pipefd[WRITE]);
        __gnu_cxx::stdio_filebuf<char> fb(pipefd[READ], std::ios::in, 1);
        std::istream is(&fb);

        // Have helper method process the output of child-process        
        sendData("text/html", pid, is, os, genChart);        
    }
}

/**
 * Process HTTP request (from first line & headers) and
 * provide suitable HTTP response back to the client.
 * 
 * @param is The input stream to read data from client.
 * @param os The output stream to send data to client.
 * @param genChart If this flag is true then generate data for chart.
 */
void serveClient(std::istream& is, std::ostream& os, bool genChart) {
    // Read headers from client and print them. This server
    // does not really process client headers
    std::string line;
    // Read the GET request line.
    std::getline(is, line);
    const std::string path = getFilePath(line);
    // Skip/ignore all the HTTP request & headers for now.
    while (std::getline(is, line) && (line != "\r")) {}
    // Check and dispatch the request appropriately
    const std::string cgiPrefix = "cgi-bin/exec?cmd=";
    const int prefixLen         = cgiPrefix.size();
    if (path.substr(0, prefixLen) == cgiPrefix) {
        // Extract the command and parameters for exec.
        const size_t argsPos   = path.find("&args=", prefixLen);
        const std::string cmd  = url_decode(path.substr(prefixLen, 
                                                        argsPos - prefixLen));
        const std::string args = url_decode(path.substr(argsPos + 6));
        // Now run the command and return result back to client.
        exec(cmd, args, os, genChart);
    } else {
        // Get the file size (if path exists)
        std::ifstream dataFile(path);
        if (!dataFile.good()) {
            // Invalid file/File not found. Return 404 error message.
            send404(os, path);
        } else {
            // Send contents of the file to the client.
            sendData(getMimeType(path), -1, dataFile, os, genChart);
        }
    }
}

//------------------------------------------------------------------
//  DO  NOT  MODIFY  CODE  BELOW  THIS  LINE
//------------------------------------------------------------------

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

/*
 * The main method that performs the basic task of accepting connections
 * from the user.
 */
int main(int argc, char** argv) {
    if (argc <= 2) {
        // Setup the port number for use by the server
        const int port = std::stoi((argc == 2 ? argv[1] : "0"));
        runServer(port);
    } else if (argc == 4) {
        // Process 1 request from specified file for functional testing
        std::ifstream input(argv[1]);
        std::ofstream output;
        if (argv[2] != std::string("std::cout")) {
            output.open(argv[2]);
        }
        bool genChart = (argv[3] == std::string("true"));
        serveClient(input, (output.is_open() ? output : std::cout), genChart);
    } else {
        std::cerr << "Invalid command-line arguments specified.\n";
    }
    return 0;
}

// End of source code
