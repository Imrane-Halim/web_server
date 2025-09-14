#include "Response.hpp"
#include <iostream>
#include <fstream>

// Utility: dump data as string, useful for testing
void dumpResponse(HTTPResponse& resp) {
    char buffer[20];
    while (!resp.isComplete()) {
        ssize_t bytes = resp.readNextChunk(buffer, sizeof(buffer));
        if (bytes < 0) {
            std::cerr << "Error reading response: " << strerror(errno) << "\n";
            break;
        }
        std::cout.write(buffer, bytes);
    }
    std::cout.flush();
}

int main() {
    // ---- Test 1: Small body response ----
    {
        std::cout << "=== Small Body Response ===\n";

        HTTPResponse resp(200, "OK");
        resp.addHeader("Content-Type", "text/plain");
        resp.setBody("Hello, world!\n");

        dumpResponse(resp);
        std::cout << "\n\n";
    }

    // ---- Test 2: File response ----
    {
        std::cout << "=== File Response ===\n";

        // Create a small test file
        const char* filename = "test_file.txt";
        {
            std::ofstream f(filename);
            f << "This is a test file served by HTTPResponse.\n";
            f << "Line 2 of the file.\n";
            f.close();
        }

        HTTPResponse resp(200, "OK");
        resp.addHeader("Content-Type", "text/plain");
        if (!resp.attachFile(filename)) {
            std::cerr << "Failed to open file: " << filename << "\n";
            return 1;
        }

        dumpResponse(resp);
        std::cout << "\n\n";
    }

    // ---- Test 3: Not Found ----
    {
        std::cout << "=== 404 Not Found ===\n";

        HTTPResponse resp(404, "Not Found");
        resp.addHeader("Content-Type", "text/html");
        resp.setBody("<html><body><h1>404 Not Found</h1></body></html>");

        dumpResponse(resp);
        std::cout << "\n\n";
    }

    return 0;
}
