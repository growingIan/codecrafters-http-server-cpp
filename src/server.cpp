#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv) {
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  
  int new_server_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected\n";

  char buffer[256];
  memset(buffer, 0, sizeof(buffer));
  int read_content = read(new_server_fd, buffer, 255);

  if (read_content < 0)
  {
    std::cout << "Error reading from socket\n";
  }
  else
  {
    std::cout << "Here is the message: \n" << buffer << "\n";
  }


  int read_pos = -1;
  for(int i = 0; buffer[i] != '\0'; i++)
  {
    if(buffer[i] == ' ')
    {
      read_pos = i+1;
      break;
    }
  }

  if (read_pos == -1)
  {
    std::cout<< "Error reading message.\n";
  }
  else
  {
    std::cout<< "Started reading message at position: " << read_pos << "\n";
  }

  std::string read_message = "";
  
  for(int i = read_pos; buffer[i] != ' '; i++)
  {
    read_message += std::string(1, buffer[i]);
  }

  std::cout<< "The read message is the following:\n";

  std::cout<< read_message << "\n";

  std::string slash = "/";

  if (read_message == slash)
  { 
    send(new_server_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  }
  else
  {
    send(new_server_fd, "HTTP/1.1 404 NOT FOUND\r\n\r\n", 26, 0);
  }

  close(new_server_fd);
  
  close(server_fd);

  return 0;
}
