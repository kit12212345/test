#include <iostream>
#include <chrono>
#include <thread>

#include "Oanda.h"

Oanda* oanda;

void StartOanda()
{
}


int main()
{

	oanda = new Oanda();

	oanda->Launch();

	int period = 30; // 30 minues interval
	
	
	while (true)
	{
		
		oanda->Cycle();
		std::this_thread::sleep_for(std::chrono::seconds(period));
	}
	

	return 0;
}