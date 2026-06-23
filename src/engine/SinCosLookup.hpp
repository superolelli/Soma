#ifndef SINCOSLOOKUP_HPP
#define SINCOSLOOKUP_HPP



#include <math.h>
#include "Patterns\Singleton.hpp"



//the class for loading all textures at once
#define g_pSinCosLookup CSinCosLookup::Get()
class CSinCosLookup : public TSingleton < CSinCosLookup >
{
public:

	//calculates all values
	void CalculateLookupTable();

	float sinLookup[361];
	float cosLookup[361];

};




#endif