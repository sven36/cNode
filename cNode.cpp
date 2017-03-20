#include "stdafx.h"
#include "node.h"


int main(int argc,wchar_t *wargv[])
{
	char** argv = new char*[argc + 1];

    return node::Start(argc,argv);
}
int Start(int argc,char** argv) {
	return 0;
}

