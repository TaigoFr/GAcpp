#pragma once

#include "Vec.hpp"
#include "Individual.hpp"

//TEMPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
#include <ctime>

class IndividualVec: public VecD, public Individual::Interface{
public:
	IndividualVec(const VecD& other): VecD(other){}
};

namespace Individual{

	template<>
	IndividualVec create<IndividualVec>(){ return IndividualVec(1).randomise(); }

	template<>
	// IndividualVec mate(const IndividualVec& I1,const IndividualVec& I2){ return (I1+I2)/2.; }
	IndividualVec mate(const IndividualVec& I1,const IndividualVec& I2){ 
		static bool b = true;
		if(b){
			b = !b;
			srand(time(NULL)+clock());
		}
		IndividualVec out(I1.size());
		for(unsigned i=0; i<out.size(); ++i)
			if(rand()<0.5*RAND_MAX)
				out[i]=I1[i];
			else
				out[i]=I2[i];
		return out; 
	}

	template<>
	double evaluate(const IndividualVec&I1){ return I1.norm()/1.; }

	template<>
	void mutate(IndividualVec& I1){ I1.randomise(); }

	template<>
	void print(IndividualVec& I1){ I1.print(); }

}

