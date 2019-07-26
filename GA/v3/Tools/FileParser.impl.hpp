
#if !defined(FILEPARSER_)
#error "This file should only be included through FileParser.h"
#endif

#include "Tools.hpp"

#include <sstream>
#include <algorithm> //std::remove

template <typename T>
bool FileParser::get(const std::string& id, T& var){
	bool found = false;

	if(!open){
		print("File ",name," not found");
		return found;
	}

	std::string line;
	while (std::getline(file, line)){

		//skip if no setter or if line is commented by "//" or "#"
		if ( line.find(setter) == std::string::npos || line.substr(0,2)=="//" || line.substr(0,1)=="#")
			continue;

		std::stringstream str(line);
		std::string id_check;

		std::getline(str, id_check, setter);

		//remove spaces and tabs
		id_check.erase(std::remove(id_check.begin(), id_check.end(), ' ' ), id_check.end());
		id_check.erase(std::remove(id_check.begin(), id_check.end(), '\t'), id_check.end());
   		
   		//verify if ID is correct
		if( id != id_check )
			continue;
   		
   		//write to variable
		str >> var;
		found = true;
		if(verbose)
			print("FOUND parameter '",id,"'. Set to value ",var);
		break;
	}
	if(!found && verbose) print("Parameter '",id,"' was NOT FOUND in file ",name);

	file.clear();
	file.seekg(0, std::ios::beg);

	return found;
}
template <typename T>
bool FileParser::operator()(const std::string& id, T& var){ return get(id, var); }

template <typename T>
bool FileParser::get(const std::string& id, T& var, const T& def){
	bool found = get(id,var);
	if(!found){
		if(verbose) print("Setting '",id,"' to its default value of ",def);
		var = def;
	}
	return found;
}
template <typename T>
bool FileParser::operator()(const std::string& id, T& var, const T& def){ return get(id, var, def); }

template <typename T>
T FileParser::get(const std::string& id, const T& def){
	T var;
	get(id,var,def);
	return var;
}
