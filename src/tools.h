#ifndef TOOLS_H
#define TOOLS_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define SPLITOR '='

using namespace std;

string getProperty(const string key, const string value, const string file);

#endif
