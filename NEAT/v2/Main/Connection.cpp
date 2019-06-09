
#include "Connection.hpp"

#include <sstream>
#include <iomanip> //std::setprecision

#include "Vec.hpp" //FORV

using namespace NEAT;

BasicConnection::BasicConnection(unsigned _pre, unsigned _pos, unsigned _innovationNumber):
pre(_pre), pos(_pos), innovationNumber(_innovationNumber)
{}

std::string BasicConnection::toString(const std::vector<BasicConnection*> &net){
	std::stringstream str;
	str << "C" << net.size() << "\n";
	FORV(i, net.size())	str << net[i]->innovationNumber << ":" << net[i]->pre << "-" << net[i]->pos << "|";
	return str.str();
}


Connection::Connection(unsigned _pre, unsigned _pos, unsigned _innovationNumber, double _weight, bool _enabled):
BasicConnection(_pre,_pos,_innovationNumber),
weight(_weight), enabled(_enabled)
{}
Connection::Connection(const std::string& str): Connection(read(str)){}

Connection Connection::read(const std::string& str){
	unsigned pre, pos, innovationNumber;
	double weight;
	bool enabled;

	std::istringstream is( str );
	is >> innovationNumber;
	is.get();
	is >> pre;
	is.get();
	is >> pos;
	is.get();
	is >> enabled;
	is.get();
	is >> weight;

	return Connection(pre, pos, innovationNumber, weight, enabled);
}


std::string Connection::toString(bool weightIfDisabled, unsigned precision) const{
	std::stringstream str;
	str << innovationNumber << ":" << pre << "-" << pos << "(" << enabled;
	if(weightIfDisabled || enabled)
		str << "=" << std::setprecision(precision) << weight;
	str << ")";
	return str.str();
}
std::string Connection::toString(const std::vector<Connection*> &net){
	std::stringstream str;
	str << "C" << net.size() << "\t";
	std::string out = str.str();
	FORV(i, net.size())	out += net[i]->toString(false,2) + "|";
	return out;
}


