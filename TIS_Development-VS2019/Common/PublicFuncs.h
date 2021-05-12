// define general public functions
#include <math.h>

inline double round(double number)
{
	return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

// Calculates log2 of number.  
inline double log2( double n )  
{  
    return log( n ) / log( 2 );  
}  