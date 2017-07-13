#include "watcher.h"

#include <iostream>

int main(int argc, char * argv[])
{
	Watcher watcher(4);
	if (!watcher.StartWatcher("127.0.0.1","9775"))
		return -1;
	return 0;
}

