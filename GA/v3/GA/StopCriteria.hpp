#pragma once

#include "Matrix.hpp"

namespace GA{

	enum StopReason
	{
		Undefined
		,MaxGenerations
		,AverageStalled
		,BestStalled
		,AverageRelativeStalled
		,BestRelativeStalled
		,BestMaximum
		,BestMinimum
	};
	void printStopReason(StopReason stop);

	class StopCriteria{
	public:
		StopCriteria(){}

		void addCriteria(StopReason stop, const VecD& params);
		void addCriteria(StopReason (*func)(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, 
						const VecD& params, bool restart), const VecD _params);

		StopReason update(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, bool restart=false);

		//parameters: {maxGenerations}
		static StopReason maxGenerations(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage,
									const VecD& params, bool restart);

		//parameters for below: {tolerance, maximum count}
		static StopReason averageStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage,
									const VecD& params, bool restart);
		static StopReason bestStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage,
			const VecD& params, bool restart);
		static StopReason averageRelativeStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage,
									const VecD& params, bool restart);
		static StopReason bestRelativeStalled(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage,
			const VecD& params, bool restart);

		//parameters: {objective}
		static StopReason bestMaximum(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage,
			const VecD& params, bool restart);
		static StopReason bestMinimum(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage,
			const VecD& params, bool restart);

		static void printStopReason(StopReason stop);

	private:
		Vec<StopReason (*)(unsigned gen, double oldBest, double newBest, double oldAverage, double newAverage, const VecD& params, bool restart)> criteria;
		MatrixD params;
	};

};