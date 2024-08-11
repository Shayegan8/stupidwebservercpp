#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define SPLITOR '='

using namespace std;

namespace T {
    inline string const getProperty(const string key, const string value, const string file) {
    ifstream config{file};
    string line{};
    if(config.is_open()) {
      while(getline(config, line)) {
        if(line.find(key) != string::npos) {
          stringstream strm{line};
          vector<string> vec{};
          string token{};
          while(getline(strm, token, SPLITOR))
            vec.push_back(token);
          return vec.back();
        }
      }
      return value;
    } else
      cerr << "\x1B[93mCan't find config or something else" << endl;
    return nullptr;
  }
}

#endif
