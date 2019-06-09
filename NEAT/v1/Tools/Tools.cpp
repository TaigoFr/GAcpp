
#include "Tools.hpp"

#include <sys/stat.h> //mkdir
#include <sstream> //ostringstream
#include <fstream> //ifstream
#include <iomanip> //std::setfill, std::setw

bool fileExists(const std::string& path){
    std::ifstream infile(path);
    return infile.good();
}

std::string checkFileName(const std::string& name, const std::string& extension){
	std::ostringstream str;
	if(name!=""){
		str << name;
		unsigned size = extension.size();
		if(name.substr( name.size()-size, size ) != extension)
			str << extension;
	}
	else{
		time_t date_t = time(NULL);
		struct tm * date = gmtime(&date_t);
		str << "NEAT_Image_";
		str << std::setw(2) << std::setfill('0') << date->tm_mday;
		str << std::setw(2) << std::setfill('0') << date->tm_mon+1;
		str << std::setw(2) << std::setfill('0') << date->tm_year-100;
		str << "_" << std::setw(2) << std::setfill('0') << date->tm_hour;
		str << std::setw(2) << std::setfill('0') << date->tm_min;
		str << std::setw(2) << std::setfill('0') << date->tm_sec;
		str << "_" << (int)clock()%1000; 
		str << extension; 
	}
	return str.str();
}