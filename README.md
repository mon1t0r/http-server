## Overview
This project is an implementation of a simple HTTP server. Its function is to
receive HTTP requests and serve the content from a local directory.

Currently supported HTTP methods are:
 - GET;
 - HEAD.

The server in its current state `IS NOT` RFC compliant, but it tries to stay
close to `RFC 9112`.

Before compilation you can configure various server options at
`include/server_config.h`. Default content directory is `html`.

## Build and run
### Requirements
```
gcc
make
cppcheck (debug build only)
```

### Build
```
git clone https://github.com/mon1t0r/http-server
cd http-server
make
```

### Run
```
release/http-server
```

### Test
Open `http://localhost:<port>/index.html` using any HTTP/1.1 client on your
machine. Replace `<port>` with your configured port number. The default port
number is `50100`.

## TODO
 - implement configuration file support;
 - implement IP address blacklist;
 - make the implementation RFC compliant;
 - change `Date` header Date-Time format to `IMF-fixdate`.

