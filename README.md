# phohttpd

A really shitty HTTP server.

By default, it serves all files from the current working directory. The directory it serves can be changes with `-d <path>`. It does not have a directory listing, if a directory is requested it simply serves `/index.html`.

## Usage

Compile with `make`.

And run with `./build/phohttpd`.
