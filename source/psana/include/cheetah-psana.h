#include <iostream>
#include <stdlib.h>

class CheetahPsana{
public:
	CheetahPsana(){
		/* If SIT_DATA is undefined set it to the builtin value */
		setenv("SIT_DATA",CHEETAH_SIT_DATA,0);
		std::cout << "Setting environment in CheetahPsana constructor" << std::endl;
	}
};

