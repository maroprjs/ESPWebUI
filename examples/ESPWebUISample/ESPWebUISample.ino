#include "Arduino.h"
#include "ESPWebUI.h"
#include "Debug.h"

ESPWebUI webUI;

void setup()
{
	//DPRINT_INIT(9600);
	webUI.begin();
}

// The loop function is called in an endless loop
void loop()
{
	webUI.handle();
}
