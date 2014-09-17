/*
 * compile :
 * g++ -std=c++11 -o maildup maildup.cc
 */

#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>

std::vector<std::string> *read_directory(std::string path) {
	std::vector<std::string> *dirlist;
	DIR *d;
	struct dirent *dir;
	d = opendir(path.c_str());
	if (d)
	{
		dirlist = new std::vector<std::string>();
		while ((dir = readdir(d)) != NULL)
		{
			if ( strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") )
			dirlist->push_back(dir->d_name);
		}
		closedir(d);
		return dirlist;
	} else return 0;
}

std::string extract_msg_id(std::string path, std::string fname) {
	static int key = 0;
	static std::string id_string = std::string("Message-ID: ");
	std::string id;
	std::ifstream file(path + "/" + fname);
	std::string line;
	int i = 0;
	//std::cout << fname << std::endl;
	while(std::getline(file, line)) {
		//std::cout << i++ << " " << line << std::endl;
		if(line.size() == 0) break;
		if( !strncasecmp(line.c_str(), id_string.c_str(), id_string.size()) ) {
			id = line.substr(id_string.size());
			//std::cout << "id=" << id << std::endl;
			return id;
		}
	}
	//std::cout << "----------------" << std::endl;
	return std::string("THISKEY") + std::to_string(key++);
}

int main(int argc, char *argv[]) {
	std::multimap <std::string, std::string> map_id_file;
	std::map <std::string, int> unique_id_list;
	std::vector<std::string> *dirlist;
	if(argc < 3) {
		std::cout << "Two parameters: input path and archive path must be given" << std::endl;
		return -1;
	}
	std::cout << "Input path:\t" << argv[1] << std::endl;
	std::cout << "Archive path:\t" << argv[2] << std::endl;
	dirlist = read_directory(argv[1]);
	if(dirlist == 0) {
		std::cout << "Could not read directory " << argv[1] << std::endl;
		return -1;
	}
	std::cout << "Input directory contains " << dirlist->size() << " files" << std::endl;
	//std::copy(dirlist->begin(), dirlist->end(), std::ostream_iterator<std::string> (std::cout,"\n"));
	for(auto it : *dirlist) {
		std::string id = extract_msg_id(argv[1], it);
		map_id_file.insert(std::pair<std::string, std::string>(id, it));
	}
	for(auto it : map_id_file) {
		unique_id_list[it.first] = 0;
	}
	std::cout << "There are " << unique_id_list.size() << " unique IDs and ";
	std::cout << dirlist->size() - unique_id_list.size() << " duplicates" << std::endl;
	for(auto uniq : unique_id_list) {
		std::cout << "-------------------------------" << std::endl;
		std::cout << uniq.first << std::endl;
		auto range = map_id_file.equal_range(uniq.first);
		int i = 0;
		for(auto it = range.first; it != range.second; ++it) {
			std::string path_old = std::string(argv[1]) + "/" + it->second;
			std::string path_new = std::string(argv[2]) + "/" + it->second;
			//std::string path_new = std::string(argv[2]) + "/";
			if(i == 0) {
				std::cout << "\t+ " << it->second << std::endl;
			}
			else {
				std::cout << "\t- " << it->second << std::endl;
				int r = mkdir(argv[2], 0755);
				r = rename(path_old.c_str(), path_new.c_str());
				if(r) {
					std::cout << "error move to " << path_new << std::endl;
				}
			}
			i ++;
		}
	}
}
