#include "Parsing.hpp"

/**
 * @brief Parses an entire web server configuration file.
 *
 * Opens the specified config file, reads it line by line, removes comments 
 * and extra spaces, and passes each line to parseAndValidateDirective() 
 * for syntax and semantic validation.
 *
 * Validates:
 *   - The file can be opened
 *   - Each line is correctly parsed and validated
 *   - Empty files are reported as errors
 *
 * @param ConfigFile Reference to WebConfigFile structure to populate with 
 *                   parsed server and location blocks
 * @param fileName   Name of the configuration file to parse
 *
 * @return 0 on success, 1 on failure (cannot open file, empty file, or syntax error)
 */
short parseConfigFile(WebConfigFile &ConfigFile, const std::string &fileName) {
    std::ifstream FileConfig(fileName.c_str());
    if (!FileConfig.is_open()) {
        std::cerr << "Error: Cannot open config.conf" << std::endl;
        return 1;
    }

    std::string line;
    size_t lineNumber = 0;
    while (std::getline(FileConfig, line)) {
        lineNumber++;
        line = removeComment(line);
        if (line.empty())
            continue;

        if (parseAndValidateDirective(line, fileName, lineNumber, ConfigFile))
            return 1;
    }

    if (!lineNumber) {
        std::cerr << "Error: file is empty" << std::endl;
        return 1;
    }

    FileConfig.close();
    return 0;
}


/**
 * @brief Parses and validates a single line from the web server configuration file.
 *
 * Tokenizes the line, manages the context (server/location blocks), 
 * routes directives to the appropriate parser, and validates syntax.
 *
 * Validates:
 *   - Prevents nested server blocks
 *   - Prevents location blocks outside a server
 *   - Ensures required fields are defined (e.g., location route)
 *   - Rejects directives outside allowed context
 *
 * Maintains static state for the current server and location while parsing.
 *
 * @param str        The configuration line to parse
 * @param fileName   Configuration file name (used for error messages)
 * @param lineNumber Line number in the configuration file
 * @param config     Reference to WebConfigFile to populate with parsed data
 *
 * @return 0 on success, 1 on syntax or context error
 */
short parseAndValidateDirective(std::string &str, const std::string &fileName, size_t &lineNumber, WebConfigFile &config) {
    static bool inServer = false;
    static bool inLocation = false;
    static Server currentServer;
    static Location currentLocation;

    std::vector<std::string> tokens = split(str);
    if (tokens.empty())
        return 0;

    if ((tokens.size() == 1 && tokens[0] == "server{") || 
        (tokens.size() == 2 && tokens[0] == "server" && tokens[1] == "{")) {
        if (inServer)
            return printError(str, fileName, lineNumber);
        inServer = true;
        currentServer = Server();
        return 0;
    }

    if ((tokens.size() == 1 && tokens[0] == "location{") || 
        (tokens.size() == 2 && tokens[0] == "location" && tokens[1] == "{")) {
        if (!inServer || inLocation)
            return printError(str, fileName, lineNumber);
        inLocation = true;
        currentLocation = Location();
        currentLocation.ApplyDefaults(currentServer);
        return 0;
    }

    if (tokens[0] == "}") {
        if (inLocation) {
            if (currentLocation.route == "")
                return printError(str, fileName, lineNumber);
            currentServer.locations.push_back(currentLocation);
            inLocation = false;
        } else if (inServer) {
            config.servers.push_back(currentServer);
            inServer = false;
        } else
            return printError(str, fileName, lineNumber);
        return 0;
    }

    if (inLocation)
        return parseLocationDirective(str, tokens, currentLocation, fileName, lineNumber);
    else if (inServer)
        return parseServerDirective(str, tokens, currentServer, fileName, lineNumber);
    else
        return printError(str, fileName, lineNumber);

    return 0;
}


/**
 * @brief Parses and validates a server-level directive from the configuration file.
 *
 * Handles directives specific to a server block, such as host, port, server_name,
 * root, client_max_body_size, index files, and custom error pages.
 *
 * Checks:
 *   - Valid IP address for host
 *   - Valid port range (0-65535)
 *   - Prevents duplicate index files
 *   - Validates error_page syntax and number
 *
 * @param str           The original line from the configuration file
 * @param tokens        Tokenized components of the line
 * @param currentServer Reference to the Server object to populate
 * @param fileName      Configuration file name (used for error reporting)
 * @param lineNumber    Line number in the configuration file
 *
 * @return 0 on success, 1 on error (syntax or invalid value)
 */
short   parseServerDirective(std::string str, std::vector<std::string> &tokens, Server &currentServer, const std::string &fileName, size_t &lineNumber) {
    if (tokens.size() < 2)
        return (printError(str, fileName, lineNumber));

    if (tokens.size() == 2 && tokens[0] == "host") {
        struct in_addr  addr;
        if (!inet_aton(tokens[1].c_str(), &addr))
            return (printError(str, fileName, lineNumber));
        currentServer.host = tokens[1];
    }

    else if (tokens.size() == 2 && tokens[0] == "port")
    {
        currentServer.port = myAtol(tokens[1], str, fileName, lineNumber);
        if (currentServer.port > 65535)
            return (printError(str, fileName, lineNumber));
    }

    else if (tokens.size() == 2 && tokens[0] == "server_name")
        currentServer.server_name = tokens[1];

    else if (tokens.size() == 2 && tokens[0] == "root")
        currentServer.root = tokens[1];

    else if (tokens.size() == 2 && tokens[0] == "client_max_body_size")
        currentServer.client_max_body_size = myAtol(tokens[1], str, fileName, lineNumber);

    else if (tokens[0] == "index") {
        currentServer.index_files.clear();
        for (size_t i = 1; i < tokens.size(); i++) {
            for (size_t j = 0; j < currentServer.index_files.size(); j++) {
                if (tokens[i] == currentServer.index_files[j])
                    return printError(str, fileName, lineNumber);
            }
            currentServer.index_files.push_back(tokens[i]);
        }
    }

    else if (tokens.size() == 3 && tokens[0] == "error_page")
        currentServer.error_pages[myAtol(tokens[1], str, fileName, lineNumber)] = tokens[2];

    else
        return (printError(str, fileName, lineNumber));

    return (0);
}


/**
 * @brief Parses and validates a location-level directive from the configuration file.
 *
 * Handles directives specific to a location block, such as route, root, autoindex,
 * client_max_body_size, redirect, upload_store, cgi_pass, index files, and allowed methods.
 *
 * Checks:
 *   - Required fields (route) are present
 *   - Autoindex is either "on" or "off"
 *   - Prevents duplicate index files
 *   - Methods are only GET, POST, or DELETE
 *
 * @param str             The original line from the configuration file
 * @param tokens          Tokenized components of the line
 * @param currentLocation Reference to the Location object to populate
 * @param fileName        Configuration file name (used for error reporting)
 * @param lineNumber      Line number in the configuration file
 *
 * @return 0 on success, 1 on syntax or validation error
 */
short   parseLocationDirective(std::string str, std::vector<std::string> &tokens, Location &currentLocation, const std::string &fileName, size_t &lineNumber) {
    if (tokens.size() < 2)
        return (printError(str, fileName, lineNumber));

    if (tokens.size() == 2 && tokens[0] == "route")
        currentLocation.route = tokens[1];

    else if (tokens.size() == 2 && tokens[0] == "root")
        currentLocation.root = tokens[1];

    else if (tokens.size() == 2 && tokens[0] == "autoindex") {
        if (tokens[1] == "on")
            currentLocation.autoindex = true;
        else if (tokens[1] == "off")
            currentLocation.autoindex = false;
        else
            return (printError(str, fileName, lineNumber));
    }

    else if (tokens.size() == 2 && tokens[0] == "client_max_body_size")
        currentLocation.client_max_body_size = myAtol(tokens[1], str, fileName, lineNumber);

    else if (tokens.size() == 2 && tokens[0] == "redirect")
        currentLocation.redirect = tokens[1];

    else if (tokens.size() == 2 && tokens[0] == "upload_store")
        currentLocation.upload_store = tokens[1];

    else if (tokens.size() == 2 && tokens[0] == "cgi_pass")
        currentLocation.cgi_pass = tokens[1];

    else if (tokens[0] == "index") {
        currentLocation.index_files.clear();
        for (size_t i = 1; i < tokens.size(); i++) {
            for (size_t j = 0; j < currentLocation.index_files.size(); j++) {
                if (tokens[i] == currentLocation.index_files[j])
                    return printError(str, fileName, lineNumber);
            }
            currentLocation.index_files.push_back(tokens[i]);
        }
    }

    else if (tokens[0] == "methods") {
        currentLocation.methods.clear();
        for (size_t i = 1; i < tokens.size(); i++) {
            for (size_t j = 0; j < currentLocation.methods.size(); j++) {
                if (tokens[i] == currentLocation.methods[j])
                    return printError(str, fileName, lineNumber);
            }

            if (tokens[i] != "GET" && tokens[i] != "POST" && tokens[i] != "DELETE")
                return printError(str, fileName, lineNumber);
            currentLocation.methods.push_back(tokens[i]);
        }
    }

    else
        return (printError(str, fileName, lineNumber));

    return (0);
}


/**
 * @brief Converts a numeric string to a size_t value, validating that all characters are digits.
 *
 * Checks that the string contains only numeric characters. If any non-digit character
 * is found, it reports a syntax error using the original line and file information.
 *
 * @param str        String to convert to a number
 * @param line       Original line from the configuration file (for error reporting)
 * @param fileName   Configuration file name (used for error reporting)
 * @param lineNumber Line number in the configuration file
 *
 * @return The converted numeric value on success, or prints an error and returns 1 on failure
 */
size_t  myAtol(std::string str, std::string &line, const std::string &fileName, size_t &lineNumber) {
    for (size_t i = 0; i < str.size(); i++) {
        if (!isdigit(str[i]))
            return (printError(line, fileName, lineNumber));
    }
    return (atol(str.c_str()));
}


/**
 * @brief Prints a syntax error message for the web server configuration.
 *
 * Outputs the file name, line number, and offending line to std::cerr,
 * and returns 1 to indicate an error.
 *
 * @param str        The line from the configuration file that caused the error
 * @param fileName   The name of the configuration file
 * @param lineNumber The line number in the configuration file
 *
 * @return Always returns 1 to indicate an error
 */
short   printError(std::string &str, const std::string &fileName, size_t &lineNumber) {
    std::cerr << "Webserv: syntax error in " << fileName << " at line " << lineNumber << " → " << str << std::endl;
    return (1);
}


/**
 * @brief Splits a string into tokens, respecting quoted substrings.
 *
 * Splits the input string by spaces, but keeps content inside single ('') or double ("") quotes together as one token.
 *
 * @param str Input string to tokenize
 *
 * @return A vector of string tokens
 */
std::vector<std::string>    split(const std::string &str) {
    std::vector<std::string>    tokens;
    std::string current;
    char    c = '\0';

    for (size_t i = 0; i < str.size();i++) {
        if ((str[i] == '\'' || str[i] == '"') && c == '\0')
            c = str[i];

        else if ((str[i] == '\'' || str[i] == '"') && c == str[i])
            c = '\0';

        else if (str[i] == ' ' && c == '\0') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
        else
            current += str[i];
    }

    if (!current.empty())
        tokens.push_back(current);

    return tokens;
}


/**
 * @brief Removes comments and extra spaces from a configuration line.
 *
 * Trims leading and trailing whitespace, removes everything after a '#' character,
 * and reduces multiple spaces or tabs to a single space.
 *
 * @param str Input line from the configuration file
 *
 * @return Cleaned line with comments and extra spaces removed
 */
std::string removeComment(const std::string &str) {
    size_t  pos = str.find('#');
    if (pos != std::string::npos)
        return (reduceSpaces(trim(str.substr(0, pos))));
    return (reduceSpaces(trim(str)));
}


/**
 * @brief Reduces multiple consecutive spaces or tabs to a single space, preserving quoted content.
 *
 * Iterates over the string and collapses consecutive whitespace characters into a single space.
 * Text inside single ('') or double ("") quotes is preserved exactly as-is.
 *
 * @param str Input string to process
 *
 * @return A string with consecutive spaces/tabs reduced to a single space
 */
std::string reduceSpaces(const std::string &str) {
    std::string result;
    bool    inSpace = false;
    char    c = '\0';

    for (size_t i = 0; i < str.size(); ++i) {
        if ((str[i] == '\'' || str[i] == '\"') && (c != '\'' && c != '"'))
            c = str[i];
        else if ((str[i] == '\'' || str[i] == '\"') && (c == str[i]))
            c = '\0';

        if ((str[i] == ' ' || str[i] == '\t') && (c != '\'' && c != '\"')) {
            if (!inSpace) {
                result += ' ';
                inSpace = true;
            }
        }
        else {
            result += str[i];
            inSpace = false;
        }
    }
    return (result);
}


/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * Removes spaces, tabs, newlines, and carriage returns from the beginning and end of the input string.
 *
 * @param str Input string to trim
 *
 * @return A trimmed string with no leading or trailing whitespace
 */
std::string trim(const std::string &str) {
    size_t  start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return ("");
    size_t  end = str.find_last_not_of(" \t\n\r");
    return (str.substr(start, end - start + 1));
}


/**
 * @brief Default constructor for the Server class.
 *
 * Initializes a server block with standard default values:
 *   - host: "127.0.0.1"
 *   - port: 80
 *   - server_name: "localhost"
 *   - root directory: "/"
 *   - default index files: ["index.html"]
 *   - maximum client body size: 10 MB
 *   - standard error pages for HTTP codes 400, 403, 404, 500
 */
Server::Server() {
    host = "127.0.0.1";
    port = 80;
    server_name = "localhost";
    root = "/";
    index_files.push_back("index.html");
    client_max_body_size = 10485760;
    error_pages[400] = "<html><body><h1>400 Bad Request</h1></body></html>";
    error_pages[403] = "<html><body><h1>403 Forbidden</h1></body></html>";
    error_pages[404] = "<html><body><h1>404 Not Found</h1></body></html>";
    error_pages[500] = "<html><body><h1>500 Internal Server Error</h1></body></html>";
}


/**
 * @brief Apply default values from the parent server to this location.
 *
 * Initializes the location with default settings inherited from the given server:
 *   - root directory
 *   - client_max_body_size
 *   - default index files
 *   - allowed methods (default: GET)
 *   - autoindex off
 *   - empty CGI, redirect, and upload_store
 *
 * @param server The parent server whose defaults are applied.
 */
void Location::ApplyDefaults(Server server) {
    route = "";
    root = server.root;
    cgi_pass = "";
    redirect = "";
    upload_store = "";
    autoindex = false;
    methods.push_back("GET");
    client_max_body_size = server.client_max_body_size;
    index_files = server.index_files;
}
