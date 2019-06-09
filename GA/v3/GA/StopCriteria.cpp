
#include "StopCriteria.hpp"
#include "Tools.hpp" //errorMsg

using namespace GA;

void StopCriteria::addCriteria(StopReason stop, const VecD& params){
	switch(stop){
		case MaxGenerations: 			addCriteria(maxGenerations, 		params); break;
		case AverageStalled:			addCriteria(averageStalled, 		params); break;
		case BestStalled:				addCriteria(bestStalled, 			params); break;
		case AverageRelativeStalled:	addCriteria(averageRelativeStalled, params); break;
		case BestRelativeStalled:		addCriteria(bestRelativeStalled, 	params); break;
		case BestMaximum:				addCriteria(bestMaximum, 			params); break;
		case BestMinimum:				addCriteria(bestMinimum, 			params); break;
		default: errorMsg("Stop reason unknown."); break;
	}
}

void StopCriteria::addCriteria(StopReason (*func)(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart), const VecD _params){
	unsigned i=0;
	for(unsigned size=criteria.size(); i<size; ++i){
		if(criteria[i]==func){
			params[i] = _params;
			break;
		}
	}
	if(i==criteria.size()){
		criteria.push_back(func);
		params.push_back(_params);
	}
}

StopReason StopCriteria::update(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, bool restart){
	if(restart){
		FORV(i,criteria.size()) criteria[i](0,0.,0.,0.,0.,{},true);
		return StopReason::Undefined;
	}
	FORV(i,criteria.size()){
		StopReason stop = criteria[i](gen, oldBest, newBest, oldAverage, newAverage, params[i], false);
		if(stop!=StopReason::Undefined) return stop;
	}
	return StopReason::Undefined;
}

StopReason StopCriteria::maxGenerations(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart){
	if(restart) return StopReason::Undefined;

	double maxGenerations = params[0];
	if(gen >= maxGenerations)
		return StopReason::MaxGenerations;
	return StopReason::Undefined;
}
StopReason StopCriteria::averageStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart){
	static unsigned averageStallCount 	= 0;
	if(restart){
		averageStallCount = 0;
		return StopReason::Undefined;
	}

	double averageStalledTol = params[0];
	unsigned averageStalledMax = params[1];

	if(fabs(oldAverage-newAverage) < averageStalledTol)
		++averageStallCount;
	else
		averageStallCount=0;

	if(averageStallCount >= averageStalledMax)
		return StopReason::AverageStalled;

	return StopReason::Undefined;
}
StopReason StopCriteria::bestStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart){
	static unsigned bestStallCount 	= 0;
	if(restart){
		bestStallCount = 0;
		return StopReason::Undefined;
	}

	double bestStalledTol = params[0];
	unsigned bestStalledMax = params[1];

	if(fabs(oldBest-newBest) < bestStalledTol)
		++bestStallCount;
	else
		bestStallCount=0;

	if(bestStallCount >= bestStalledMax)
		return StopReason::BestStalled;

	return StopReason::Undefined;
}
StopReason StopCriteria::averageRelativeStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart){
	static unsigned averageStallCount 	= 0;
	if(restart){
		averageStallCount = 0;
		return StopReason::Undefined;
	}

	double averageRelativeStalledTol = params[0];
	unsigned averageRelativeStalledMax = params[1];

	if(fabs((oldAverage-newAverage)/oldAverage) < averageRelativeStalledTol)
		++averageStallCount;
	else
		averageStallCount=0;

	if(averageStallCount >= averageRelativeStalledMax)
		return StopReason::AverageRelativeStalled;

	return StopReason::Undefined;
}
StopReason StopCriteria::bestRelativeStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart){
	static unsigned bestStallCount 	= 0;
	if(restart){
		bestStallCount = 0;
		return StopReason::Undefined;
	}

	double bestRelativeStalledTol = params[0];
	unsigned bestRelativeStalledMax = params[1];

	if(fabs((oldBest-newBest)/oldBest) < bestRelativeStalledTol)
		++bestStallCount;
	else
		bestStallCount=0;

	if(bestStallCount >= bestRelativeStalledMax)
		return StopReason::BestRelativeStalled;

	return StopReason::Undefined;
}
StopReason StopCriteria::bestMaximum(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart){
	if(restart) return StopReason::Undefined;

	double maximum = params[0];
	if(newBest >= maximum)
		return StopReason::BestMaximum;
	return StopReason::Undefined;
}
StopReason StopCriteria::bestMinimum(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart){
	if(restart) return StopReason::Undefined;

	double minimum = params[0];
	if(newBest >= minimum)
		return StopReason::BestMinimum;
	return StopReason::Undefined;
}

void StopCriteria::printStopReason(StopReason stop){
	std::string str;
	switch(stop)
	{
		case StopReason::Undefined:
			str = "No-stop";
			break;
		case StopReason::MaxGenerations:
			str = "Maximum generation reached";
			break;
		case StopReason::AverageStalled:
			str = "Average stalled";
			break;
		case StopReason::BestStalled:
			str = "Best stalled";
			break;
		case StopReason::AverageRelativeStalled:
			str = "Relative Average stalled";
			break;
		case StopReason::BestRelativeStalled:
			str = "Relative Best stalled";
			break;
		case StopReason::BestMaximum:
			str = "Best Maximum reached";
			break;
		case StopReason::BestMinimum:
			str = "Best Minimum reached";
			break;
		default:
			str = "Unknown reason";
	}

	std::cout << "Stop criteria: ";
	if(stop == StopReason::Undefined)
		std::cout << "There is a bug in this function";
	else
		std::cout << str;
	std::cout << std::endl;
}