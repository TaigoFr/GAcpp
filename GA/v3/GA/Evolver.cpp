
#include "Evolver.hpp"

Random GA::generator;

void GA::printStopReason(StopReason stop){
	std::string str;
	switch(stop)
	{
		case StopReason::Undefined:
		str = "No-stop";
		break;
		case StopReason::MaxGenerations:
		str = "Maximum generation reached";
		break;
		case StopReason::StallAverage:
		str = "Average stalled";
		break;
		case StopReason::StallBest:
		str = "Best stalled";
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