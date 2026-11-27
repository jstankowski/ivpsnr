#include "xSplatOps.h"
#include "fmt/format.h"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <random>
#include <ctime>
#include <numeric>

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

int32 xSplatOps::xSphHarmCountFromDegree(int32 Degree)
{
	assert(Degree >= 0);

	if (Degree == 0) { return 0; }
	else { return xSphHarmCountFromDegree(Degree - 1) + (Degree * 2) + 1; }

	//probably better to use "switch based LUT" intead of recursive function

	//switch(Degree)
	//{
	//case 0: return 0 ;
	//case 1: return 3 ;
	//case 2: return 8 ;
	//case 3: return 15;
	//default: assert(0);
	//}
}
int32 xSplatOps::xSphHarmDegreeFromCount(int32 Count)
{
	switch (Count)
	{
	case  0: return 0;
	case  3: return 1;
	case  8: return 2;
	case 15: return 3;
	default: assert(0); return NOT_VALID;
	}
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB