#include "node.h"


int wmain(int argc,wchar_t *wargv[])
{
	char** argv = new char*[argc + 1];
    return node::Start(argc,argv);
}


