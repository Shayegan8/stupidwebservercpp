#include "tools.h"

#include <csignal>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <future>
#include <memory>
#include <ostream>
#include <sys/socket.h>
#include <cstdlib>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <netdb.h>
#include <netinet/in.h>
#include <malloc.h>
#include <string>
#include <iostream>

#define MAX_BUFF 2048
#define MIN_BUFF 128

using namespace std;

struct Properties {
  
  string welcome;
  string host;
  string port;
  string clients;
};

void err(const string str);
string cmd(const string cmd);
extern string const filler(string str);
char* const_tomut(const char* str);

int main() {
  ostringstream strm{};
  strm << getenv("HOME") << "/.local/webserver/server.conf";
  cout << strm.str();
  
  Properties props{getProperty("welcome", "\n\x1B[91mHello bitch\x1B[93m", strm.str()), getProperty("host", "0.0.0.0", strm.str()), getProperty("port", "12341", strm.str()), getProperty("clients", "100", strm.str())};
  cout << props.welcome << endl;
  
  int sockfd {socket(PF_INET, SOCK_STREAM, 0)};
  if(sockfd < 0) 
    err("cant make socket");
  
  struct sockaddr_in sockaddri;
  struct hostent* host;

  sockaddri.sin_port = htons(stoi(props.port));
  sockaddri.sin_family = AF_INET;
  host = gethostbyname(props.host.c_str());
  sockaddri.sin_addr = *(struct in_addr*) host->h_addr;

  if(bind(sockfd, (struct sockaddr*) &sockaddri, sizeof(sockaddri)) < 0)
    err("cant bind");

  if(listen(sockfd, stoi(props.clients)) < 0)
    err("cant listen");

  auto inp = [sockfd, sockaddri]() {
    while(EXIT_FAILURE) {
      string str{};
      cin >> str;
      if(str.compare("quit") == 0) {
        close(sockfd);
        cout << "\x1B[0m" << endl;
        exit(EXIT_SUCCESS); 
      } else if(str.compare("memory") == 0) {
        cout << "\x1B[95m";
        malloc_stats();
        cout << endl;
      }
    }             
  };

  future<void> futu = async(launch::async, inp);

  cout << "connection established\nIP: " << props.host << ":" << props.port << endl;

  signal(SIGINT, [](int sigc){
    cout << "\ntype quit to get out" << endl;         
  });
  
  string response {
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"};

  string splitor{"  "};
  ostringstream path{};
  path << "dir " << getenv("HOME") << "/.local/webserver/files/html/";
  string files {cmd(path.str())};

  vector<string> m{};

  char *c_st {const_tomut(files.c_str())};
  char* contx{nullptr};
  char* str_save{nullptr};
  
  if(strstr(files.c_str(), " ") != NULL) {
    while((str_save = strtok_r(c_st, "  ", &contx)) != NULL) {
      
      char* temp{str_save};
      char* rstr_save{nullptr};
      if((rstr_save = strtok_r(temp, ".", &rstr_save)) != NULL)
        m.push_back(string(rstr_save));

      c_st = nullptr;
    }
  } else
    if((str_save = strtok_r(c_st, ".", &str_save)) != NULL)
      m.push_back(string(str_save));
  
  auto handle = [sockfd, response, sockaddri, props, m]() {
    while (EXIT_FAILURE) {
      struct sockaddr_in client_address; 
      socklen_t client_len = sizeof(client_address);                   
      int clientfd {accept(sockfd, (struct sockaddr *)&client_address, &client_len)};
      if (clientfd < 0)
        continue;
      char buffer[MAX_BUFF];     
      memset(buffer, '\0', sizeof(buffer));
      read(clientfd, buffer, sizeof(buffer) - 1);
      
      char ip[MAX_BUFF];
      cout << "Connection accepted\n" << inet_ntop(AF_INET, &sockaddri.sin_addr.s_addr, ip, sizeof(ip)) << " => " << buffer << endl; 

      string real_resp(response);

      bool is_it {false};
      string real_str{};
      for(const auto &entry: m) {
        ostringstream cs{};
        cs << "GET /" << entry.c_str() << " HTTP/1.1";
        if(strstr(buffer, cs.str().c_str()) != NULL) {
          is_it = true;
          real_str = entry.c_str();
        }
      }
      
      if(is_it) {
        cout << "loading" << endl;
        ostringstream fis{};
        fis << getenv("HOME") << "/.local/webserver/files/html/" << real_str << ".html";
        cout << fis.str() << endl;
        ifstream ifi{fis.str()};     
          if(ifi.is_open()) {
            string line{};
            while(getline(ifi, line)) {
              real_resp += line;
              real_resp += "\n";
            }
          real_resp += "\r\n";
          cout << real_resp << endl;
          write(clientfd, real_resp.c_str(), real_resp.size());
          } else {
            string responsi{"HTTP/1.1 404 Not Found\r\n""Content-Type: text/html\r\n""Connection: close\r\n""\r\n""<h3>PAGE NOT FOUND - SWSERVER 1.0.0</h3>"};
            write(clientfd, responsi.c_str(), responsi.size());
          }        
      } else {
        string responsi{"HTTP/1.1 404 Not Found\r\n""Content-Type: text/html\r\n""Connection: close\r\n""\r\n""<h3>PAGE NOT FOUND - SWSERVER </h3>"};
        write(clientfd, responsi.c_str(), responsi.size());
      }
    close(clientfd);
    cout << "closed" << endl;
    }
  };
  future<void> ahandl = async(launch::async, handle);
}

void err(const string str) {
  
  cout << str << "\x1B[0m" << endl;
  exit(EXIT_FAILURE);
}

string cmd(string cmd) {
  
  string result{};
  char buffer[MAX_BUFF];
  unique_ptr<FILE, decltype(&pclose)> pid(popen(cmd.c_str(), "r"), pclose);

  while(fgets(buffer, sizeof(buffer), pid.get()) != NULL)
    result += buffer;
  
  return result;  
}

char* const_tomut(const char* str) {
  
  size_t length = strlen(str) + 1;
  char* newstr = new char[length];
  strncpy(newstr, str, length);
  newstr[length - 1] = '\0';
  
  return newstr;
}
