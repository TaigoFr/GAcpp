#if !defined(_INDIVIDUAL_)
#error "This file should only be included through Individial.hpp"
#endif

template <typename Ind>
Individual<Ind>::Individual(): 
					create(nullptr), mate(nullptr), mutate(mutate_default), evaluate(nullptr), print(print_default)
					,fitness(0.){}
	
template <typename Ind> void Individual<Ind>::setCreate	(Ind 	(*func)())						{	 create 	= func; }
template <typename Ind> void Individual<Ind>::setMate 	(Ind 	(*func)(const Ind&,const Ind&));{	 mate   	= func; }
template <typename Ind> void Individual<Ind>::setMutate	(void 	(*func)(Ind&));					{	 mutate 	= func; }
template <typename Ind> void Individual<Ind>::setEvaluate(double (*func)(const Ind&));			{	 evaluate 	= func; }
template <typename Ind> void Individual<Ind>::setPrint	(void 	(*func)(Ind&));					{	 print 		= func; }

template <typename Ind>	Ind Individual<Ind>::create	(){
	if(create == nullptr)
		throw std::runtime_error("Please define 'Ind create()' via setCreate(*).\n"):
}
template <typename Ind>
	Ind 	mate 	(const Ind&,const Ind&);
template <typename Ind>
void Individual<Ind>::mutate	(Ind&);
template <typename Ind>
	double	evaluate(const Ind&);
template <typename Ind>
void Individual<Ind>::print	(Ind&);

template <typename Ind>
	double getFitness()			{ return fitness; }
template <typename Ind>
void Individual<Ind>::setFitness(double f)	{ fitness = f; }