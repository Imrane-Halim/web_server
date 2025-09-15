#include "Parsing.hpp"

void printConfigFile(const WebConfigFile &config);

/**
 * @brief Entry point for the web server program.
 *
 * Expects exactly one argument: the path to the configuration file.
 * Parses the configuration file and populates the WebConfigFile object.
 * Returns 1 on error (invalid arguments or parsing failure), 0 on success.
 *
 * @param ac Number of command-line arguments
 * @param av Array of command-line arguments
 * @param env Environment variables (currently unused)
 * @return int Exit status (0 on success, 1 on error)
 */
int main(int ac, char **av, char **env) {
    if (ac != 2) {
        std::cerr << "usage: ./webserv [CONFIG]" << std::endl;
        return (1);
    }

    WebConfigFile ConfigFile;

    if (parseConfigFile(ConfigFile, av[1]))
        return (1);

    printConfigFile(ConfigFile);

    (void) ac;
    (void) av;
    (void) env;

    return (0);
}


/**
 * @brief Print the entire web server configuration.
 *
 * Iterates over all servers and their locations, printing each directive
 * in a readable format.
 *
 * @param config The WebConfigFile object to print
 */
void printConfigFile(const WebConfigFile &config)
{
    for (size_t i = 0; i < config.servers.size(); ++i) {
        const Server &srv = config.servers[i];
        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  Host: " << srv.host << "\n";
        std::cout << "  Port: " << srv.port << "\n";
        std::cout << "  Server Name: " << srv.server_name << "\n";
        std::cout << "  Root: " << srv.root << "\n";
        std::cout << "  Client Max Body Size: " << srv.client_max_body_size << "\n";

        std::cout << "  Index files: ";
        for (size_t j = 0; j < srv.index_files.size(); ++j)
            std::cout << srv.index_files[j] << " ";
        std::cout << "\n";

        std::cout << "  Error Pages:\n";
        for (std::map<int, std::string>::const_iterator it = srv.error_pages.begin(); it != srv.error_pages.end(); ++it)
            std::cout << "    " << it->first << " -> " << it->second << "\n";

        std::cout << "  Locations:\n";
        for (size_t k = 0; k < srv.locations.size(); ++k) {
            const Location &loc = srv.locations[k];
            std::cout << "    Location " << k + 1 << ":\n";
            std::cout << "      Route: " << loc.route << "\n";
            std::cout << "      Root: " << loc.root << "\n";
            std::cout << "      Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
            std::cout << "      Redirect: " << loc.redirect << "\n";
            std::cout << "      Upload Store: " << loc.upload_store << "\n";
            std::cout << "      Client Max Body Size: " << loc.client_max_body_size << "\n";
            std::cout << "      CGI Pass: " << loc.cgi_pass << "\n";

            std::cout << "      Methods: ";
            for (size_t m = 0; m < loc.methods.size(); ++m)
                std::cout << loc.methods[m] << " ";
            std::cout << "\n";

            std::cout << "      Index files: ";
            for (size_t n = 0; n < loc.index_files.size(); ++n)
                std::cout << loc.index_files[n] << " ";
            std::cout << "\n";
        }
        std::cout << "----------------------------------------\n";
    }
}
