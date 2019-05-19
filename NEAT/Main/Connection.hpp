#pragma once

#include <vector>
#include <string>

namespace NEAT {
	
	// Describes the connection between two nodes
	class BasicConnection{
	public:
		BasicConnection(unsigned _pre = 0, unsigned _pos = 0, unsigned _innovationNumber = 0);

		static std::string toString(const std::vector<BasicConnection*> &net);

		const unsigned pre, pos;
		const unsigned innovationNumber;
	};

	class Connection: public BasicConnection{
	public:
		Connection(unsigned _pre, unsigned _pos, unsigned _innovationNumber, double _weight, bool _enabled);
		Connection(const std::string& str);
		// Connection(const Connection& con);

		static Connection read(const std::string& str);

		std::string toString(bool weightIfDisabled, unsigned precision) const;
		static std::string toString(const std::vector<Connection*> &net);

		inline static bool compare(const Connection* c1, const Connection* c2){ return c1->innovationNumber < c2->innovationNumber; }

		inline bool getEnabled() const{ return enabled; }
		inline void enable() { enabled = true; }
		inline void disable(){ enabled = false; }

		double weight;
	private:
		// Node indices
		bool enabled;
	};

}