```markdown
# HTTP Server  

A multi-part project for building an HTTP server from scratch. This project covers request parsing, socket communication, and static file serving while providing hands-on experience with the fundamentals of the HTTP protocol, client-server interactions, and multi-threaded server implementations.  

## Features  
- Parse HTTP requests to extract headers, methods, paths, and payloads.  
- Handle HTTP requests over sockets for client-server communication.  
- Serve static files with appropriate HTTP response codes and content types.  
- Threaded server architecture for handling multiple clients.  

## Getting Started  

### Prerequisites  
- C compiler (e.g., `gcc`)  
- Python 3.x (for testing Part 3 with `pytest`)  

### Cloning the Repository  
Clone the repository to your local machine:  
```bash  
git clone https://github.com/abbasmirza10/http-server-mp5.git  
cd http-server  
```  

## Implementation Details  

### Part 1: Parsing an HTTP Request  
- Implement the functions in `http.c`, excluding `httprequest_read`.  
- Parse HTTP request headers and populate the `HTTPRequest` structure.  
- Test with:  
  ```bash  
  make tester  
  ./tester "[part=1]"  
  ```  

### Part 2: Reading HTTP Request from a Socket  
- Complete the `httprequest_read` function in `http.c`.  
- Efficiently read HTTP request data from a socket file descriptor.  
- Test with:  
  ```bash  
  ./tester "[part=2]"  
  ```  

### Part 3: Building the Web Server  
- Implement the `client_thread` function in `server.c`.  
- Serve static files based on the requested path.  
- Respond with appropriate HTTP status codes (`200 OK` or `404 Not Found`).  
- Test by running:  
  ```bash  
  make  
  ./staticserver 34000  
  ```  
  Then, open a browser and visit:  
  - [http://localhost:34000/](http://localhost:34000/)  
  - [http://localhost:34000/microbe.html](http://localhost:34000/microbe.html)  
  - [http://localhost:34000/340.png](http://localhost:34000/340.png)  

## Testing  
- **Parts 1 & 2:** Use the provided test suite.  
- **Part 3:** Manually test by visiting the server using a web browser.   
```
