#pragma once

#include <string>
#include <vector>
using namespace std;

string getFileContent(string file);
vector<string> getFiles(string ext = "xs");
void saveToFile(string f, string s);
bool sameToFile(string f, string s);
bool isUTF8(string content);