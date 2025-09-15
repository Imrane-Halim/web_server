#include <iostream>
#include <algorithm>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

class   WebConfigFile;
class   Server;
class   Location;

/**
 * @class WebConfigFile
 * @brief Represents the complete web server configuration file.
 *
 * This class stores all server blocks defined in a web server configuration file,
 * including all their associated location blocks.
 *
 * @var servers
 * @brief Vector containing all Server objects defined in the configuration.
 */
class WebConfigFile {
public:
    std::vector<Server> servers;
};


/**
 * @class Server
 * @brief Represents a server block in the web server configuration.
 *
 * This class stores all configuration for a single server, including its listening IP and port,
 * server name, default root directory, maximum client body size, default index files, error pages,
 * and all associated location blocks.
 *
 * @note Use the default constructor Server() to initialize a server with standard default values.
 *
 * @var host
 * @brief IP address that the server listens on.
 *
 * @var port
 * @brief Port number that the server listens on.
 *
 * @var server_name
 * @brief Server name used for virtual hosting.
 *
 * @var root
 * @brief Default root directory for the server.
 *
 * @var client_max_body_size
 * @brief Maximum size (in bytes) allowed for client request bodies.
 *
 * @var index_files
 * @brief List of default index files for the server (e.g., index.html).
 *
 * @var error_pages
 * @brief Mapping of HTTP status codes to custom error page file paths.
 *
 * @var locations
 * @brief Vector of all Location blocks associated with this server.
 *
 * @fn Server
 * @brief Default constructor that initializes all server fields with standard defaults.
 */
class Server {
    public:
        std::string host;
        int port;
        std::string server_name;
        std::string root;
        size_t client_max_body_size;
        std::vector<std::string> index_files;
        std::map<int, std::string> error_pages;
        std::vector<Location> locations;

        Server();
};



/**
 * @class Location
 * @brief Represents a location block in the web server configuration.
 *
 * This class stores all configuration related to a specific URL path within a server block.
 * It includes settings for the directory root, allowed HTTP methods, maximum client body size,
 * index files, autoindex behavior, CGI paths, redirects, and upload storage.
 *
 * @note Use ApplyDefaults(Server server) to initialize a location with default values from
 *       the parent server block.
 *
 * @var route
 * @brief The URL path that this location block handles.
 *
 * @var root
 * @brief The root directory for this location.
 *
 * @var client_max_body_size
 * @brief Maximum size (in bytes) allowed for client request bodies for this location.
 *
 * @var index_files
 * @brief List of default index files for this location (e.g., index.html).
 *
 * @var methods
 * @brief Allowed HTTP methods (e.g., GET, POST, DELETE) for this location.
 *
 * @var autoindex
 * @brief Flag indicating whether directory listing is enabled (true) or disabled (false).
 *
 * @var upload_store
 * @brief Directory path where uploaded files will be stored.
 *
 * @var redirect
 * @brief URL path to redirect requests to (if set).
 *
 * @var cgi_pass
 * @brief Path to CGI executable for handling dynamic requests.
 *
 * @fn ApplyDefaults
 * @brief Initializes the location block with default values from the parent server.
 * @param server The parent Server object to copy defaults from.
 */
class Location {
    public:
        std::string route;
        std::string root;
        size_t client_max_body_size;
        std::vector<std::string> index_files;
        std::vector<std::string> methods;
        bool autoindex;
        std::string upload_store;
        std::string redirect;
        std::string cgi_pass;

        void ApplyDefaults(Server server);
};



// Removes leading and trailing whitespace from a string
std::string trim(const std::string &str);

// Removes comments (after '#') and trims extra spaces
std::string removeComment(const std::string &str);

// Reduces consecutive spaces/tabs to a single space (preserves quoted text)
std::string reduceSpaces(const std::string &str);

// Splits a string into tokens by spaces, keeping quoted strings intact
std::vector<std::string> split(const std::string &str);

// Prints standardized syntax error message and returns 1
short printError(std::string &str, const std::string &fileName, size_t &lineNumber);

// Parses entire configuration file and fills WebConfigFile structure
short parseConfigFile(WebConfigFile &ConfigurationFile, const std::string &fileName);

// Converts numeric string to size_t; returns error if non-digit found
size_t myAtol(std::string str, std::string &line, const std::string &fileName, size_t &lineNumber);

// Parses and validates a single directive line from the config
short parseAndValidateDirective(std::string &str, const std::string &fileName, size_t &lineNumber, WebConfigFile &config);

// Parses directives inside a server block and updates Server object
short parseServerDirective(std::string str, std::vector<std::string> &tokens, Server &currentServer, const std::string &fileName, size_t &lineNumber);

// Parses directives inside a location block and updates Location object
short parseLocationDirective(std::string str, std::vector<std::string> &tokens, Location &currentLocation, const std::string &fileName, size_t &lineNumber);
