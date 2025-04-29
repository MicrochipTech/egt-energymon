#include <egt/ui>
#include <egt/themes/lapis.h>
#include <iostream>
#include "mainwin.h"

using namespace std;
using namespace egt;
using namespace egt::experimental;

int main(int argc, const char ** argv)
{
	Application app;

	mainWin window;

	window.show();

	return app.run();
}



