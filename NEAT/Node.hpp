
#pragma once

#include <vector>
#include <memory>

#include "Connection.hpp"

namespace NEAT{

class Node{
public:
	Node(double _bias): bias(_bias){}
	Node(const Node&&) = delete;
	Node(const Node&)  = delete; //just to make sure

	// void addPreConnection(const std::shared_ptr<Connection> &new_connection){
		// pre_connections.push_back(new_connection);
	// }
	// void addPosConnection(){ ++pos_connetions; }

	void addBias(double b){ bias += b; }
	// unsigned numConnections() const{ return pre_connections.size() + pos_connetions; }

	// bool contains(unsigned n) const{
	// 	for(auto it = pre_connections.begin(); it<pre_connections.end(); ++it)
	// 		if((*it)->pre == n)
	// 			return true;
	// 	return false;
	// }

private:
	// std::vector< std::shared_ptr<Connection> > pre_connections;
	// unsigned pos_connetions;
	double bias;
};

}