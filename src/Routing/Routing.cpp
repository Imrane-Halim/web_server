#include "Routing.hpp"

/**
 * @brief Constructs a Routing object with a reference to a WebConfigFile.
 *
 * This constructor initializes the Routing instance by storing a reference
 * to the configuration object (_config), which holds all server and location
 * information. The Routing object will use this configuration to route
 * HTTP requests to the appropriate server.
 *
 * @param config Reference to the WebConfigFile containing server configurations.
 */
Routing::Routing(ServerConfig &server) : _server(server)
{
}

/**
 * @brief Finds the Location that best matches the request path.
 *
 * Returns the Location with the longest route that matches the beginning
 * of the request path. If no match is found, returns NULL.
 *
 * @param server The server containing the locations.
 * @param request_path The HTTP request path to match.
 * @return Pointer to the best matching Location, or NULL if none found.
 */
Location *Routing::findLocation(ServerConfig &server, const std::string &request_path)
{
    Location *best = NULL;
    size_t best_len = 0;

    for (size_t i = 0; i < server.locations.size(); i++)
    {
        Location &loc = server.locations[i];
        if (request_path.compare(0, loc.route.length(), loc.route) == 0)
        {
            if (loc.route.length() > best_len)
            {
                best = &loc;
                best_len = loc.route.length();
            }
        }
    }
    return (best);
}

/**
 * @brief Checks if the given HTTP method is allowed for a specific Location.
 *
 * This function iterates through all methods defined in the Location object
 * and returns true if the provided method matches any of the allowed methods.
 *
 * @param loc The Location object containing allowed HTTP methods.
 * @param method The HTTP method from the client request to check.
 * @return true if the method is allowed for this Location, false otherwise.
 */
bool Routing::isMethodAllowed(Location &loc, const std::string &method)
{
    for (size_t i = 0; i < loc.methods.size(); i++)
    {
        if (loc.methods[i] == method)
            return (true);
    }
    return (false);
}

/**
 * @brief Determines the best routing match for an incoming HTTP request.
 *
 * This function performs the full routing resolution process:
 *  1. Finds the ServerConfig that matches the provided host name.
 *  2. Finds the best matching Location within that server based on the request path.
 *  3. Checks whether the given HTTP method is allowed for that Location.
 *
 * The results are returned in a RouteMatch structure, which contains:
 *  - `sv`: pointer to the matched ServerConfig (or NULL if not found)
 *  - `lc`: pointer to the matched Location (or NULL if not found)
 *  - `method`: boolean indicating whether the HTTP method is allowed
 *
 * If either the server or location is not found, or the method is not allowed,
 * the corresponding fields in the returned RouteMatch will reflect that.
 *
 * @param host The "Host" header value from the HTTP request.
 * @param request_path The path portion of the HTTP request URL.
 * @param method The HTTP method used in the request (e.g., "GET", "POST", "DELETE").
 * @return A RouteMatch structure containing the resolved routing information.
 */
RouteMatch Routing::getMatch(const std::string &request_path, const std::string &method)
{
    RouteMatch res;

    res.sv = &_server;
    res.lc = NULL;
    res.m = false;

    if (!(res.lc = findLocation(*res.sv, request_path)))
        return (res);

    res.m = isMethodAllowed(*res.lc, method);

    return (res);
}
