#include "SinCosLookup.hpp"



void CSinCosLookup::CalculateLookupTable()
{
	for (int i = 0; i < 361; i++)
	{
		sinLookup[i] = sin(i*3.1415926535 / 180);
		cosLookup[i] = cos(i*3.1415926535 / 180);
	}
}