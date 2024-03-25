#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <fstream>

void socket_work(int new_server_fd, std::string dir = "")
{
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
  std::string full_message = "";
  
  for(int i = read_pos; buffer[i] != ' '; i++)
  {
    read_message += std::string(1, buffer[i]);
  }

  for(int i = read_pos; buffer[i] != '\0'; i++)
  {
    full_message += std::string(1, buffer[i]);
  }

  std::cout<< "The read message is the following:\n";

  std::cout<< read_message << "\n";

  std::string slash = "/";
  std::string echo = "/echo/";
  std::string user_agent = "/user-agent";
  std::string files = "/files/";

  if (read_message == slash)
  { 
    send(new_server_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  }
  else if (read_message.size() >= 6 && read_message.substr(0, 6) == echo)
  {
    char out_buffer[512];
    memset(out_buffer, 0, sizeof(out_buffer));
    std::string message = read_message.substr(6, (int)read_message.size()-6);
    std::string response_prefix = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "; 
    size_t response_content_length = message.size();
    response_prefix += std::to_string(response_content_length);
    response_prefix += "\r\n\r\n";

    for(int i = 0; i<response_prefix.size(); i++)
    {
      out_buffer[i] = response_prefix[i];
    }

    for(int i = response_prefix.size(); i<response_prefix.size() + message.size(); i++)
    {
      out_buffer[i] = message[i - (int)response_prefix.size()];
    }

    out_buffer[response_prefix.size() + message.size()] = '\0';

    std::cout<< "The response being sent is:\n" << response_prefix + message << "\n";

    send(new_server_fd, out_buffer,  response_prefix.size() + message.size() + 1, 0);
  }
  else if (read_message.size() >= 11 && read_message.substr(0, 11) == user_agent)
  {
    std::string user_agent_string = "User-Agent: ";
    size_t pos = full_message.find(user_agent_string);

    if (pos != std::string::npos)
    {
      std::cout<< "Found User-Agent: in request, starts at position: " << pos << "\n";
    }

    std::string message = "";

    for(int i = pos + user_agent_string.size(); full_message[i] != '\r' && full_message[i] != '\0'; i++)
    {
      message += std::string(1, full_message[i]);
    }

    std::cout<< "User-Agent string is the following:\n";
    std::cout<< message << "\n";

    char out_buffer[512];
    memset(out_buffer, 0, sizeof(out_buffer));
    std::string response_prefix = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "; 
    size_t response_content_length = message.size();
    response_prefix += std::to_string(response_content_length);
    response_prefix += "\r\n\r\n";

    for(int i = 0; i<response_prefix.size(); i++)
    {
      out_buffer[i] = response_prefix[i];
    }

    for(int i = response_prefix.size(); i<response_prefix.size() + message.size(); i++)
    {
      out_buffer[i] = message[i - (int)response_prefix.size()];
    }

    out_buffer[response_prefix.size() + message.size()] = '\0';

    std::cout<< "The response being sent is:\n" << response_prefix + message << "\n";

    send(new_server_fd, out_buffer,  response_prefix.size() + message.size() + 1, 0);

  }
  else if (read_message.size() >= 7 && read_message.substr(0, 7) == files)
  {
    char out_buffer[4096];
    memset(out_buffer, 0, sizeof(out_buffer));
    
    std::string filename = read_message.substr(7, (int)read_message.size() - 7);

    std::cout << "The filename is: \n";
    std::cout << filename <<" \n";

    std::string full_file_path = dir + "/" + filename;

    std::cout << "The full file path is: \n";
    std::cout << full_file_path <<" \n";

    std::ifstream file;

    file.open(full_file_path.c_str(), std::ios::binary);

    std::string response_prefix;
    std::string message;

    if (!file.is_open())
    {
      send(new_server_fd, "HTTP/1.1 404 NOT FOUND\r\n\r\n", 26, 0);
    }
    else
    { 
      char ch;

      while(!file.eof())
      {
        ch = file.get();
        message += std::string(1, ch);
      }

      file.close();

      response_prefix = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: "; 
      size_t response_content_length = message.size();
      response_prefix += std::to_string(response_content_length-1);
      response_prefix += "\r\n\r\n";
    }

    for(int i = 0; i<response_prefix.size(); i++)
    {
      out_buffer[i] = response_prefix[i];
    }

    for(int i = response_prefix.size(); i<response_prefix.size() + message.size(); i++)
    {
      out_buffer[i] = message[i - (int)response_prefix.size()];
    }

    out_buffer[response_prefix.size() + message.size()] = '\0';

    std::cout<< "The response being sent is:\n" << response_prefix + message << "\n";

    send(new_server_fd, out_buffer,  response_prefix.size() + message.size() + 1, 0);
  }
  else
  {
    send(new_server_fd, "HTTP/1.1 404 NOT FOUND\r\n\r\n", 26, 0);
  }

  close(new_server_fd);
}

int main(int argc, char **argv) {
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  std::string dir = "";

  if (argc > 1)
  {
    for(int i = 0; argv[2][i] != '\0'; i++)
    {
      dir += std::string(1, argv[2][i]);
    }

    std::cout << "The directory passed is: \n";
    std::cout << dir << "\n";
  }

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

  while(true)
  {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    int new_server_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    std::cout << "Client connected\n";

    std::thread t(socket_work, new_server_fd, dir);
    t.detach();
  } 
  
  close(server_fd);

  return 0;
}
