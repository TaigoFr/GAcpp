//testing binary search
//after while, 'pos' equals the upper bound in 'array' for a given 'target' (and 'N' if the target is past the max value of the array)

#include <cstdio>

int main(){

	const unsigned N = 10;
	double array[N] = {1,2,3,4,5,6,7,8,9,10};

	double target = 4.5;

	unsigned pos = 0;
    unsigned limit = N;
    while(pos < limit)
    {
        unsigned testpos = pos+((limit-pos)>>1);

        if (array[testpos] < target)
            pos=testpos+1;
        else
            limit=testpos;
    }

    printf("\npos=%u\nlimit=%u\n\n",pos,limit);

    // return (pos < length && array[pos]==valueToFind);
	
	return 0;
}