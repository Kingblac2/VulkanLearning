#include"app.h"
#include<vulkan/vulkan.h>

#pragma comment(lib,"vulkan-1.lib")
int main()
{
	App* myApp = new App(640, 480, true);
	
	myApp->run();
	delete myApp;
	return 0;
}