#include <iostream>
#include <stdlib.h>

class CheetahPsana{
public:
	CheetahPsana(){
		/* If SIT_DATA is undefined set it to the builtin value */
		setenv("SIT_DATA",CHEETAH_SIT_DATA,0);
		/* Set the git commit sha1 on an environment variable so we can compare from the modules */
		unsetenv("PSANA_GIT_SHA");
		setenv("PSANA_GIT_SHA",GIT_SHA1,0);
		std::cout << "Setting environment in CheetahPsana constructor" << std::endl;
	}
};

