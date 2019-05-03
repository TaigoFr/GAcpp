#pragma once

#include <SFML/Graphics.hpp>

#include "Matrix.hpp"
#include "Evolver.hpp"
#include "Network.hpp"

namespace NEAT{

	class Image: public GA::Evolver<MatrixD>{
	public:
		Image(const Network&, unsigned _populationSize, double _margin = 0.03);

		void evolve(unsigned _maxGenerations, unsigned eliteCount, double crossoverProb, double mutateProb);
		void print(); //prints positions to command line
		void draw(float screenWidth = 1200.f, float screenHeight = 900.f);  //display SFMl image of network
		void wait();  //wait until SFML window is closed
		void close(); //force SFML window to close
		void save(const std::string& name = ""); //save image to file

		//functions for GA::Evolver
		static MatrixD 		create 	 (const GA::Evolver<MatrixD>*);
		static MatrixD 		crossover(const MatrixD&,const MatrixD&, double fit1, double fit2);
		static void 		mutate	 (MatrixD&, const GA::Evolver<MatrixD>*);
		static double		evaluate (const MatrixD&, const GA::Evolver<MatrixD>*);
		static std::string	toString (const MatrixD&);

	private:
		// const unsigned numInputs, numOutputs, numNodes;
		const Network* net;
		double margin;
		double maxMutation;
		double connectionAttractionPower;
		double nodeRepulsionPower;
		double nodeRepulsionCoeff;

		sf::RenderWindow window;

		static double nodeDistance(const MatrixD&, unsigned n1, unsigned n2, double power); //n1 and n2 are nodes, not matrix entries (node 0 is bias and is not in matrix)
		static sf::RectangleShape createRect(float x1, float y1, float x2, float y2, float thickness, sf::Color color);
	};
};