/**
 * Copyright (C) overbetn@miamioh.edu 
 * 
 * Simple mutli threaded web server that simulates a stock market. Allows user
 * to add stocks, raise stock, lower stocks and check the status of stocks. 
 */

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <iomanip>
#include <vector>

// Setup a server socket to accept connections on the socket
using namespace boost::asio;
using namespace boost::asio::ip;

// Alias to an unordered_map that holds account number and balance.
using Market = std::unordered_map<std::string, double>;

// A garbage-collected container to hold client I/O stream in threads
using TcpStreamPtr = std::shared_ptr<tcp::iostream>;

// A shared mutex stored in the namespace used to lock the critical sections
// of the program. 
namespace mt {
    std::mutex stonkLock;
}
/**
 * Parses URL and grabs the command, stock ticker and price, does not need all
 * three.
 * 
 * @param is - The input stream from the socket created by the server
 * @return std::vector - returns a vector of arguments provided by the GET
 *                       request.
 */
std::vector<std::string> parseRequest(std::istream &is) {
    std::string line, fLine, dummy;
    std::vector<std::string> args;
    // Grabs first line (line we care about)
    getline(is, fLine);
    while (getline(is, dummy)) {
        if (dummy.find("Connection: close\r") == std::string::npos) {
            continue;
        } else {
            getline(is, dummy);
            break;
        }
    }
    // Create string stream to parse that line.
    std::istringstream ss(fLine);
    ss >> line >> line;
    std::istringstream ls(line);
    getline(ls, dummy, '=');
    while (getline(ls, dummy, '&')) {
        args.push_back(dummy);
        getline(ls, dummy, '=');
    }
    return args;
}
/**
 * This method is called when the server is ready to send a response to the 
 * client and simply prints out the respective HTTP response header for the 
 * given GET request. 
 * 
 * @param response - Message to be sent after header.
 * @param os - Output socket to write to the client.
 */
void sendResponse(const std::string response, std::ostream &os) {
    os << "HTTP/1.1 200 OK\r\n"
       << "Server: StockServer\r\n"
       << "Content-Length: " << response.length() << "\r\n"
       << "Connection: Close\r\n"
       << "Content-Type: text/plain\r\n\r\n" << response;
}

/**
 * This method simply processes the data and performs the respective necessary
 * operations when the ticker is not found in the unordered map.
 * 
 * @param args - Vector that is holding all the necessary arguments from the
 *               GET request.
 * @param stonks - Map containing all the stock tickers mapped to their 
 *                 respective stock prices.
 */
void process1(const std::vector<std::string> &args, Market &stonks) {
    if (args.at(0) == "create") {
        stonks[args.at(1)] = 0;
    }
}
/**
 * This method simply processes the data and performs the respective necessary
 * operations when the ticker is not found in the unordered map.
 * 
 * @param args - Vector that is holding all the necessary arguments from the
 *               GET request.
 * @param stonks - Map containing all the stock tickers mapped to their 
 *                 respective stock prices.
 * @param ss - This is an output string stream that is sending to the respond
 *             string which contains the message to be sent back by the server
 */
void getResponse1(const std::vector<std::string> &args, Market &stonks,
        std::ostringstream &ss) {
    if (args.at(0) == "create") {
        ss << "Stock " << args.at(1) << " created";
    } else if (args.at(0) == "up" || args.at(0) == "down" \
            || args.at(0) == "status") {
        ss << "Stock not found";
    }
}
/**
 * This method simply processes the data and performs the respective necessary
 * operations when the ticker is found in the unordered map.
 * 
 * @param args - Vector that is holding all the necessary arguments from the
 *               GET request.
 * @param stonks - Map containing all the stock tickers mapped to their 
 *                 respective stock prices.
 */
void process2(const std::vector<std::string> &args, Market &stonks) {
    if (args.at(0) == "up") {
        stonks[args.at(1)] += std::stod(args.at(2));
    } else if (args.at(0) == "down") {
        stonks[args.at(1)] -= std::stod(args.at(2));
    }
}
/**
 * This method simply gets the respective responses when the server does contain
 * an entry of a given stock ticker. 
 * 
 * @param args - Vector that is holding all the necessary arguments from the
 *               GET request.
 * @param stonks - Map containing all the stock tickers mapped to their 
 *                 respective stock prices.
 * @param ss - This is an output string stream that is sending to the respond
 *             string which contains the message to be sent back by the server
 */
void getResponse2(const std::vector<std::string> &args, Market &stonks,
        std::ostringstream &ss) {
    if (args.at(0) == "create") {
        ss << "Stock " << args.at(1) << " already exists";
    } else if (args.at(0) == "up" || args.at(0) == "down") {
        ss << "Stock price updated";
    } else if (args.at(0) == "status") {
        ss << "Stock " << args.at(1) << ": $" << std::fixed 
           << std::setprecision(2) << stonks.at(args.at(1));
    }
}
/**
 * After the HTTP header has been properly parsed this method will take the 
 * commands from that header and start to find out what to execute on the 
 * stocks map. 
 * 
 * @param args - Vector that is holding all the necessary arguments from the
 *               GET request.
 * @param stonks - Map containing all the stock tickers mapped to their 
 *                 respective stock prices.
 * @param response - This is a string that stores the message to be sent by the
 *                   server after it has sent all of the respective headers. 
 */
void processRequest(const std::vector<std::string> &args,
        Market &stonks, std::string &response) {
    std::ostringstream ss(response);   
    if (args.at(0) == "reset") {
        stonks.clear();
        ss << "All stocks reset";
    } else if (stonks.find(args.at(1)) == stonks.end()) {
        // stonkLock.lock();
        process1(args, stonks);
        // stonkLock.unlock();
        getResponse1(args, stonks, ss);
        // stonkLock.unlock();
    } else {
        // stonkLock.lock();
        process2(args, stonks);
        getResponse2(args, stonks, ss);
        // stonkLock.unlock();
    }
    response = ss.str();
}
/**
 * This method is where the program starts to read in the request headers sent
 * by the client, parses those headers, and then executes the commands given
 * in that GET request.
 * 
 * @param is - The input stream from the socket created by the server
 * @param os - The output stream from the socket created by the server
 * @param stonks - Map of stocks (strings) mapped to stock price (double)
 */
void serveClient(std::istream& is, std::ostream& os, Market& stonks) {
    // Send the URL to parse URL to know what actions to take. String -> Double
    // Each request will be standard GET request, each line terminated by /r/n
    // so simple string processing to parse the request. Only care about the
    // first line of GET request to grab URL info.
    const std::vector<std::string> args = parseRequest(is);
    // store stock ticker mapped to price with unordered_map.
    const std::vector<std::string> vec = {"reset", "create", "up", "down",
                                      "status"};
    if (!args.empty()) {
        for (size_t i = 0; i < vec.size(); i++) {
            std::string response;
            if (args.at(0) == vec[i]) {
                std::lock_guard<std::mutex> guard(mt::stonkLock);
                processRequest(args, stonks, response);
                sendResponse(response, os);
                break;
            } else if (args.at(0) != vec[i] && i == vec.size() - 1) {
                response = "Invalid request";
                sendResponse(response, os);
                break;
            }
        }        
    }
}
/**
 * Top-level method to run a custom HTTP server to process bank
 * transaction requests using multiple threads. Each request should
 * be processed using a separate detached thread. This method just loops 
 * for-ever.
 *
 * @param server The boost::tcp::acceptor object to be used to accept
 * connections from various clients.
 */
void runServer(tcp::acceptor& server) {
    // Implement this method to meet the requirements of the
    // homework. See earlier labs on using detached threads with
    // web-server for examples.
    Market stonks;
    while (true) {
        auto client = std::make_shared<tcp::iostream>();
        // Wait for a client to connect
        server.accept(*client->rdbuf());
        // Process information from client.
        std::thread thr([client, &stonks]() {
            serveClient(*client, *client, stonks); });
            thr.detach();
    }
    
    // Needless to say first you should create stubs for the various 
    // operations, write comments, and then implement the methods.
    //
    // First get the base case operational. Submit it via CODE for
    // extra testing. Then you can work on the multithreading case.
}

//-------------------------------------------------------------------
//  DO  NOT   MODIFY  CODE  BELOW  THIS  LINE
//-------------------------------------------------------------------

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

// Helper method for testing.
void checkRunClient(const std::string& port);

/*
 * The main method that performs the basic task of accepting
 * connections from the user and processing each request using
 * multiple threads.
 */
int main(int argc, char** argv) {
    // Setup the port number for use by the server
    const int port = (argc > 1 ? std::stoi(argv[1]) : 0);
    io_service service;
    // Create end point.  If port is zero a random port will be set
    tcp::endpoint myEndpoint(tcp::v4(), port);
    tcp::acceptor server(service, myEndpoint);  // create a server socket
    // Print information where the server is operating.    
    std::cout << "Listening for commands on port "
              << server.local_endpoint().port() << std::endl;
    // Check run tester client.
#ifdef TEST_CLIENT
    checkRunClient(argv[1]);
#endif

    // Run the server on the specified acceptor
    runServer(server);
    
    // All done.
    return 0;
}

// End of source code

