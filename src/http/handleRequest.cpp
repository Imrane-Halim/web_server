#include "ConfigParser.hpp"
#include "Routing.hpp"
#include "HTTPParser.hpp"
#include "Response.hpp"
#include "error_pages.hpp"

/**
 * @brief Create a standardized HTTP error response for a given status code.
 *
 * This function centralizes the creation of error pages, ensuring consistency
 * across the server. It uses the global defaultErrorPages map to retrieve
 * the HTML body corresponding to the provided HTTP status code.
 *
 * @param code The HTTP status code to generate the response for (e.g., 404, 500).
 * @return HTTPResponse Fully constructed HTTPResponse object with headers and body.
 *
 * @details
 * Steps performed in this function:
 * 1. Construct an HTTPResponse object with the given status code.
 * 2. Add the "Content-Type: text/html" header so the client knows the content is HTML.
 * 3. Set the body of the response using getErrorPage(code), which returns
 *    the HTML page corresponding to the status code.
 * 4. Mark the end of headers with endHeaders() so the response is ready to be sent.
 */
HTTPResponse createErrorResponse(int code)
{
    cout << "Erroreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee: [" << code << "]" << endl; // tmp

    HTTPResponse resp(code);
    resp.addHeader("Content-Type", "text/html");
    resp.setBody(getErrorPage(code));
    resp.endHeaders();
    return (resp);
}

/**
 * @brief Handles an incoming HTTP request and generates the appropriate response.
 *
 * This function performs the full request routing and response generation logic,
 * including server lookup, location resolution, method validation, redirects, and
 * static file serving.
 *
 * @param routing The Routing object containing server and location configurations.
 * @param host The "Host" header value from the HTTP request, used to find the server.
 * @param request_path The requested URI/path.
 * @param method The HTTP method of the request (e.g., GET, POST, DELETE).
 * @return HTTPResponse The constructed response ready to be sent to the client.
 *
 * @details
 * Steps performed in this function:
 * 1. Find the server that matches the requested host using routing.findServer().
 *    - If no server matches, return a 404 Not Found error response.
 *
 * 2. Find the location within the server that matches the request path using routing.findLocation().
 *    - If no location matches, return a 404 Not Found error response.
 *
 * 3. Check if the HTTP method is allowed for this location using routing.isMethodAllowed().
 *    - If not allowed, return a 405 Method Not Allowed error response.
 *
 * 4. Check if the location has a redirect configured (loc->redirect is not empty).
 *    - If yes, generate a 301 Moved Permanently response with the "Location" header
 *      pointing to the new URL. The client will follow this redirect automatically.
 *
 * 5. Serve static files:
 *    - Construct the full file path by combining loc->root and request_path.
 *    - Attempt to attach the file to the response using resp.attachFile().
 *    - If the file does not exist or cannot be opened, return a 404 Not Found error response.
 *
 * 6. Add the "Content-Type: text/html" header and mark the end of headers with endHeaders().
 *    - This ensures the response is complete and ready to be sent.
 */
HTTPResponse handleRequest(Routing &routing, const string &host, const string &request_path, const string &method)
{
    ServerConfig *server = routing.findServer(host);
    if (!server)
        return (createErrorResponse(404));

    Location *loc = routing.findLocation(*server, request_path);
    if (!loc)
        return (createErrorResponse(404));

    if (!routing.isMethodAllowed(*loc, method))
        return (createErrorResponse(405));

    if (!loc->redirect.empty())
    {
        HTTPResponse resp(301, "Moved Permanently");
        resp.addHeader("Location", loc->redirect);
        resp.endHeaders();
        return (resp);
    }

    string filepath = loc->root + request_path;
    HTTPResponse resp;
    if (!resp.attachFile(filepath))
        return (createErrorResponse(404));

    cout << "Good Requesteeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << endl; // tmp

    resp.addHeader("Content-Type", "text/html");
    resp.endHeaders();
    return (resp);
}
