// clang-format off

static char error_400_page[] =
    "<!DOCTYPE html> \
    <html> \
        <head> \
            <style> \
                body { \
                    padding-left: 15px; \
                    background-color: #111111; \
                    color: white; \
                } \
                footer { \
                    font-style: italic; \
                    font-size: 14px; \
                } \
            </style> \
        </head> \
        <body> \
            <h1>400 Bad Request</h1> \
            <p>The client's request was invalid.</p> \
            <footer>phohttpd</footer> \
        </body> \
    </html> \
    ";

static char error_403_page[] =
    "<!DOCTYPE html> \
    <html> \
        <head> \
            <style> \
                body { \
                    padding-left: 15px; \
                    background-color: #111111; \
                    color: white; \
                } \
                footer { \
                    font-style: italic; \
                    font-size: 14px; \
                } \
            </style> \
        </head> \
        <body> \
            <h1>403 Forbidden</h1> \
            <p>You do not have permission to access this file.</p> \
            <footer>phohttpd</footer> \
        </body> \
    </html> \
    ";

static char error_404_page[] =
    "<!DOCTYPE html> \
    <html> \
        <head> \
            <style> \
                body { \
                    padding-left: 15px; \
                    background-color: #111111; \
                    color: white; \
                } \
                footer { \
                    font-style: italic; \
                    font-size: 14px; \
                } \
            </style> \
        </head> \
        <body> \
            <h1>404 Not Found</h1> \
            <p>The requested file was not found.</p> \
            <footer>phohttpd</footer> \
        </body> \
    </html> \
    ";

static char error_500_page[] =
    "<!DOCTYPE html> \
    <html> \
        <head> \
            <style> \
                body { \
                    padding-left: 15px; \
                    background-color: #111111; \
                    color: white; \
                } \
                footer { \
                    font-style: italic; \
                    font-size: 14px; \
                } \
            </style> \
        </head> \
        <body> \
            <h1>500 Internal Server Error</h1> \
            <p>The server encountered an error while processing your request.</p> \
            <footer>phohttpd</footer> \
        </body> \
    </html> \
    ";
