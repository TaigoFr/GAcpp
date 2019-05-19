
#include <cstdio>

class A{
public:
	A(double _x = 0.): x(_x){ printf("In A(double)\n"); }
	A(const A& a): x(a.x){ printf("In A(const A&)\n"); }

	double x;
};

A func(){
	A a;
	return a;
}

int main(){

	// A a = func();

	// A b;
	// b = func();

	// A c(func());

	A* d = NULL;
	d = new A(func());

	delete d;

	double aij[2][25] = {{-32,16,0,16,32, -32,16,0,16,32, -32,16,0,16,32, -32,16,0,16,32, -32,16,0,16,32},{-32,-32,-32,-32,-32, -16,-16,-16,-16,-16, 0,0,0,0,0, 32,32,32,32,32, 16,16,16,16,16}};
	printf("[20,1]=%lf",aij[1][20]);

	return 0;
}