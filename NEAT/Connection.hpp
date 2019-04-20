#pragma once

//TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <iostream>

namespace NEAT {
	
// Describes the connection between two nodes
class BasicConnection{
public:
	BasicConnection(unsigned _pre, unsigned _pos, unsigned _innovationNumber):
		pre(_pre), pos(_pos), innovationNumber(_innovationNumber)
		{}

	const unsigned pre, pos;
	const unsigned innovationNumber;
};

class Connection: public BasicConnection{
public:
	Connection(unsigned _pre, unsigned _pos, unsigned _innovationNumber, double _weight, bool _enabled):
		BasicConnection(_pre,_pos,_innovationNumber),
		weight(_weight), enabled(_enabled)
		{}

	static bool compare(const std::shared_ptr<Connection> &c1, const std::shared_ptr<Connection> &c2){ return c1->innovationNumber < c2->innovationNumber; }

	void addWeight(double w){ weight += w; }

	bool getEnabled(){ return enabled; }
	void disable(){ enabled = false; }
private:
	// Node indices
	double weight;
	bool enabled;
};

}