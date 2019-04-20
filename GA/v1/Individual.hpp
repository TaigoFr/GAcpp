#pragma once

namespace Individual{

	class Interface{
	public:
		Interface(): fitness(0.){}
		double getFitness()			{ return fitness; }
		void   setFitness(double f)	{ fitness = f; }
	private:
		double fitness;
	};

	template <typename Ind>
	Ind 	create	();

	template <typename Ind>
	Ind 	mate 	(const Ind&,const Ind&);

	template <typename Ind>
	double	evaluate(const Ind&);

	template <typename Ind>
	void 	mutate	(Ind&) { return; }

	template <typename Ind>
	void 	print	(Ind&) { return; }

};