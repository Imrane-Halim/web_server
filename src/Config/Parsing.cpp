#include "Parsing.hpp"

short parseConfigFile(WebConfigFile &config, const string &fname) {
    ifstream inputFile(fname.c_str());
    if (!inputFile.is_open()) {
        cerr << "Error: Cannot open config file " << fname << endl;
        return (1);
    }
    string currentLine;
    size_t lnNbr = 0;
    while (getline(inputFile, currentLine)) {
        ++lnNbr;
        currentLine = removeComment(currentLine);
        if (currentLine.empty())
            continue;

        if (handleDirective(currentLine, fname, lnNbr, config))
            return (1);
    }
    if (lnNbr == 0) {
        cerr << "Error: Configuration file is empty" << endl;
        return (1);
    }
    inputFile.close();
    return (0);
}

string removeComment(const string &str) {

    (void)str;
}

short handleDirective(string &str, const string &fname, size_t &lnNbr, WebConfigFile &config) {

    (void)str;
    (void)fname;
    (void)lnNbr;
    (void)config;
    return (0);
}

Server::Server() {
    ip = "127.0.0.1";
    port = 80;
    name = "localip";
    root = "/";
    files.push_back("index.html");
    maxBody = 10485760;
    errors[400] = "<html><body><h1>400 Bad Request</h1></body></html>";
    errors[403] = "<html><body><h1>403 Forbidden</h1></body></html>";
    errors[404] = "<html><body><h1>404 Not Found</h1></body></html>";
    errors[500] = "<html><body><h1>500 Internal Server Error</h1></body></html>";
}

void Location::ApplyDefaults(Server server) {
    route = "";
    root = server.root;
    cgi = "";
    redirect = "";
    upload = "";
    autoindex = false;
    methods.push_back("GET");
    maxBody = server.maxBody;
    files = server.files;
}
