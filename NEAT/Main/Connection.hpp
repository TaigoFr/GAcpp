#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iomanip> //std::setprecision

//TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <iostream>

namespace NEAT {
	
// Describes the connection between two nodes
class BasicConnection{
public:
	BasicConnection(unsigned _pre, unsigned _pos, unsigned _innovationNumber):
		pre(_pre), pos(_pos), innovationNumber(_innovationNumber)
		{}

	static std::string toString(const std::vector<BasicConnection*> &net){
		std::stringstream str;
		str << "C" << net.size() << "\n";
		for(unsigned i=0, size=net.size(); i<size; ++i)
			str << net[i]->innovationNumber << ":" << net[i]->pre << "-" << net[i]->pos << "|";
		return str.str();
	}

	const unsigned pre, pos;
	const unsigned innovationNumber;
};

class Connection: public BasicConnection{
public:
	Connection(unsigned _pre, unsigned _pos, unsigned _innovationNumber, double _weight, bool _enabled):
		BasicConnection(_pre,_pos,_innovationNumber),
		weight(_weight), enabled(_enabled)
		{}

	static std::string toString(const std::vector<Connection*> &net){
		std::stringstream str;
		str << "C" << net.size() << "\t";
		for(unsigned i=0, size=net.size(); i<size; ++i){
			str << net[i]->innovationNumber << ":" << net[i]->pre << "-" << net[i]->pos << "(" << net[i]->getEnabled();
			if(net[i]->getEnabled())
				str << "=" << std::setprecision(2) << net[i]->weight;
			str << ")|";
		}
		return str.str();
	}

	static bool compare(const Connection* c1, const Connection* c2){ return c1->innovationNumber < c2->innovationNumber; }

	bool getEnabled(){ return enabled; }
	void enable() { enabled = true; }
	void disable(){ enabled = false; }
	
	double weight;
private:
	// Node indices
	bool enabled;
};

}