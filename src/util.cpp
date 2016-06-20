#include "util.h"
#include "lang.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#endif


string getFileContent(string file) {
	fstream fs(file, ios::in);
	if (fs.is_open()) {
		stringstream ss;
		char ch;
		while (fs.peek() != EOF) {
			fs.read(&ch, 1);
			ss << ch;
		}
		fs.close();
		return ss.str();
	}
	return "";
}

vector<string> getFiles(string ext) {
	auto list = vector<string>();
#ifdef _WIN32
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	hFind = FindFirstFileA(".\\*", &ffd);
	if (INVALID_HANDLE_VALUE == hFind) return list;
	do {
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			string f = ffd.cFileName;
			size_t pos = f.find_last_of('.');
			if (ext != f.substr(pos + 1, f.length())) continue;
			list.push_back(f);
		}
	} while (FindNextFileA(hFind, &ffd) != 0);
	FindClose(hFind);
#else
	DIR *dp = opendir("./");
	if (!dp) return list;
	
	struct dirent *entry;
	struct stat statbuf;
	while ((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) continue;
		string s(entry->d_name);
		size_t pos = s.find_last_of('.');
		if (ext != s.substr(pos + 1, s.length())) continue;
		list.push_back(entry->d_name);
	}
	closedir(dp);
#endif
	return list;
}

void saveToFile(string f, string s) {
	fstream fs(f, ios::out);
	fs << s;
	fs.close();
}

bool sameToFile(string f, string s) {
	return getFileContent(f) == s;
}
