
#include "Image.hpp"
#include "Node.hpp"
#include "Tools.hpp"

using namespace NEAT;

#include <cmath> //powf
#include <ctime> //time, gmtime, clock
#include <iomanip> //std::setfill, std::setw

Image::Image(const Network& net, const std::string& file, const std::string& prefix): GA::Evolver<MatrixD>(file,prefix)
,net(&net)
//set the defaults either here or directly in fileImageSettings (better here to avoid more output)
,margin(0.03)
,maxMutation(0.1)
,connectionAttractionPower(2.)
,nodeRepulsionPower(-1.)
,nodeRepulsionCoeff(6./4.)
{
	verbose = false;
	fileImageSettings(file,prefix);

	setCreate(Image::create);
	setCrossover(Image::crossover);
	setMutate(Image::mutate);
	setEvaluate(Image::evaluate, GA::MINIMIZE);
	setToString(Image::toString);
}
Image::Image(const Network& net, unsigned _populationSize, double _margin):
GA::Evolver<MatrixD>(_populationSize, -1, false)
// ,numInputs(net.getNumInputs())
// ,numOutputs(net.getNumOutputs())
// ,numNodes(net.getNumNodes())
,net(&net)
,margin(_margin)
,maxMutation(0.1)
,connectionAttractionPower(2.)
,nodeRepulsionPower(-1.)
,nodeRepulsionCoeff(6./4.)
{
	setCreate(Image::create);
	setCrossover(Image::crossover);
	setMutate(Image::mutate);
	setEvaluate(Image::evaluate, GA::MINIMIZE);
	setToString(Image::toString);
}

void Image::fileImageSettings(const std::string& file, const std::string& prefix){
	FileParser fp(file, false);

	if(!fp(prefix+"margin",margin))
		errorMsg("margin is a mandatory parameter.");
	fp(prefix+"maxMutation",maxMutation);
	fp(prefix+"margin",margin);
	fp(prefix+"connectionAttractionPower",connectionAttractionPower);
	fp(prefix+"nodeRepulsionPower",nodeRepulsionPower);
	fp(prefix+"nodeRepulsionCoeff",nodeRepulsionCoeff);
}

void Image::evolve(){ GA::Evolver<MatrixD>::evolve(); }
void Image::evolve(unsigned _maxGenerations, unsigned eliteCount, double crossoverProb, double mutateProb){
	addCriteria(GA::StopReason::MaxGenerations,{(double)_maxGenerations});
	GA::Evolver<MatrixD>::evolve(eliteCount, crossoverProb, mutateProb);
}

MatrixD Image::create(const GA::Evolver<MatrixD>* ev){
	unsigned numNodes 	= ((Image*)ev)->net->getNumNodes();
	unsigned numInputs 	= ((Image*)ev)->net->getNumInputs();
	unsigned numOutputs = ((Image*)ev)->net->getNumOutputs();
	double margin = ((Image*)ev)->margin;

	MatrixD out(numNodes-1, 3); //-1 cause bias is not displayed; 3 being [x,y,bias]

	FORV(i,1,numInputs+1){
		out[i-1][0] = margin;
		out[i-1][1] = margin + i*(1.-2.*margin)/(numInputs+1);
		out[i-1][2] = 0.; //no bias for input
	}
	FORV(i,1,numOutputs+1){
		out[i-1+numInputs][0] = 1.-margin;
		out[i-1+numInputs][1] = margin + i*(1.-2.*margin)/(numOutputs+1);
		out[i-1+numInputs][2] = ((Image*)ev)->net->getBias(i + numInputs);
	}
	static double epsilon = 1e-3; //to avoid overlap with input or output
	FOR(i,1+numInputs+numOutputs, numNodes){
		out[i-1][0] = GA::generator(2.*margin + epsilon, 1. - 2.*margin - epsilon);
		out[i-1][1] = GA::generator(2.*margin, 1. - 2.*margin);
		out[i-1][2] = ((Image*)ev)->net->getBias(i);
	}

	return out;
}
MatrixD Image::crossover(const MatrixD& p1,const MatrixD& p2, double, double){
	if(p1.getNL() != p2.getNL() || p1.getNC() != p2.getNC())
		errorMsg("Parent matrices of different sizes");

	MatrixD out(p1.getNL(),p2.getNC());

	unsigned crossoverPoint = GA::generator.getI(0,p1.getNL());
	FORV(i,p1.getNL()){
		//will re-mix inputs and outputs, but it's ok, they're all the same
		if(i < crossoverPoint)
			out[i] = p1[i];
		else
			out[i] = p2[i];
	}
	return out;
}
void Image::mutate(MatrixD& mat, const GA::Evolver<MatrixD>* ev){
	static double epsilon   = 1e-3; //to avoid overlap with input or output
	double maxMutation = ((Image*)ev)->maxMutation;

	unsigned numInputs 	= ((Image*)ev)->net->getNumInputs();
	unsigned numOutputs = ((Image*)ev)->net->getNumOutputs();
	double margin = ((Image*)ev)->margin;

	FORV(i,numInputs+numOutputs, mat.getNL()){
		//only mutate hidden nodes (recall that bias is not in 'mat')
		mat[i][0] += GA::generator(-maxMutation,maxMutation);
		if(mat[i][0]<2.*margin+epsilon)
			mat[i][0] = 2.*margin+epsilon;
		else if(mat[i][0]>1.-(2.*margin+epsilon))
			mat[i][0] = 1.-(2.*margin+epsilon);

		mat[i][1] += GA::generator(-maxMutation,maxMutation);
		if(mat[i][1]<margin)
			mat[i][1] = 2.*margin;
		else if(mat[i][1]>1.-2.*margin)
			mat[i][1] = 1.-2.*margin;
	}
}
double Image::evaluate(const MatrixD& mat, const GA::Evolver<MatrixD>* ev){
	const std::vector<Connection*> &connections = ((Image*)ev)->net->getConnections();

	// unsigned numInputs 	= ((Image*)ev)->net->getNumInputs();
	// unsigned numOutputs = ((Image*)ev)->net->getNumOutputs();

	// bool extraRepulsion = true;
	double attraction = 0.;
	FORV(i, connections.size()){
		if(connections[i]->pre==0 || !connections[i]->getEnabled()) continue; //bias connection
		// if(connections[i]->pre!=0 && connections[i]->pre<=numInputs && connections[i]->pos > numInputs && connections[i]->pos <= numInputs+numOutputs)
			// extraRepulsion = true;
		attraction += nodeDistance(mat,connections[i]->pre,connections[i]->pos,((Image*)ev)->connectionAttractionPower);
	}

	double repulsion = 0.;
	for(unsigned i=1, numNodes=mat.getNL()+1; i<numNodes; ++i)
		FOR(j,i+1,numNodes)	repulsion += nodeDistance(mat,i,j,((Image*)ev)->nodeRepulsionPower);

	unsigned numHidden 	= ((Image*)ev)->net->getNumHidden();
	repulsion *= ((Image*)ev)->nodeRepulsionCoeff * 1. / pow(sqrt(numHidden)+1, 3); //see Tests2.nb Mathematica Notebook
	// if(extraRepulsion) repulsion *= 6.; //repulse more if inputs are connected to outputs

	return attraction + repulsion;
}

double Image::nodeDistance(const MatrixD& mat, unsigned n1, unsigned n2, double power){
	if(n1==0 || n2==0 || n1 > mat.getNL() || n2 > mat.getNL())
		errorMsg("Invalid node.");

	double x = mat[n1-1][0] - mat[n2-1][0];
	double y = mat[n1-1][1] - mat[n2-1][1];
	double norm2 = x*x + y*y;
	if(power==2)
		return norm2;
	if(power==0)
		return 1;
	if(norm2==0)
		return (power<0 ? 1.e10 : 0.);
	return pow(norm2,power/2.);
}

void Image::print() const{ ::print(toString(getBest())); }
void Image::draw(float screenWidth, float screenHeight){
	window.create(sf::VideoMode(screenWidth, screenHeight), "Visualizer");
	window.clear(sf::Color::White);
	
	// display SFMl image of network
	const std::vector<Connection*> &connections = net->getConnections();
	const MatrixD& nodes = getBest();

	//ATTENTION - node '0' is 'bias' node. Matrix 'nodes' starts on node 1!

	FORV(i, connections.size()){
		unsigned pre = connections[i]->pre;
		if(pre==0) continue; //bias

		unsigned pos = connections[i]->pos;

		float color = Node::sigmoid(connections[i]->weight);
		float mod = atan( fabs(connections[i]->weight)/4.f ) * 2.f / M_PI; 											//empirically tested
		sf::RectangleShape rect = createRect(nodes[pre - 1][0] * screenWidth, nodes[pre - 1][1] * screenHeight,
			nodes[pos - 1][0] * screenWidth, nodes[pos - 1][1] * screenHeight,
											 6.f * ( 0.1 + 0.9 * mod ),												//empirically tested
											 sf::Color((1 - color) * 255, color * 255, 0, 255*(0.3+0.3*mod))); 		//empirically tested
		window.draw(rect);
	}

	sf::CircleShape circle;
	float radius = (exp( - (net->getNumNodes() * net->getNumNodes() / 500.f) ) * 0.8 + 0.2) * 20.f; 				//experimentally tested
	circle.setRadius(radius);
	circle.setOrigin(radius, radius);
	circle.setOutlineColor(sf::Color::Black);
	circle.setOutlineThickness(1.f);

	FORV(i,nodes.getNL()){
		circle.setPosition(nodes[i][0] * screenWidth, nodes[i][1] * screenHeight);

		float color = Node::sigmoid(nodes[i][2]);
		circle.setFillColor(sf::Color((1 - color) * 255, color * 255, 100));

		window.draw(circle);
	}

	window.display();
}

void Image::handleEvent(){
	window.pollEvent(event);
	if (event.type == sf::Event::Closed || 
		(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Return)))
		close();
}

void Image::wait(){
	while (window.isOpen())
		handleEvent();
}

void Image::save(const std::string& name) const{
	std::string path = checkFileName(name,".png");
	
	sf::Texture texture;
	texture.create(window.getSize().x, window.getSize().y);
	texture.update(window);
	texture.copyToImage().saveToFile(path);
}


sf::RectangleShape Image::createRect(float x1, float y1, float x2, float y2, float thickness, sf::Color color){
	sf::RectangleShape rect;
	rect.setPosition(x1, y1);
	rect.setSize(sf::Vector2f(std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)), thickness));
	rect.setFillColor(color);
	rect.setRotation(atan2(y2 - y1, x2 - x1) * 180.f / M_PI);
	return rect;
}