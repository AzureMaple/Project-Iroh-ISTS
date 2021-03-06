#ifndef IROH_IROHLIBRARY_IROH_BEACON_H
#define IROH_IROHLIBRARY_IROH_BEACON_H

#include <Windows.h>

#include <string>

#include "../Utilities/obfuscate.h"

#define DEFAULT_CONTROLSERVER	(wchar_t *)(char *)AY_OBFUSCATE("localhost")
#define DEFAULT_PORTNUMBER		6969
#define DEFAULT_SLEEPTIMER		15000

namespace Iroh {

	class Beacon
	{
	public:
		// C++ Class constructor.
		Beacon();

	private:
		std::wstring controlServer;
		INT port;
		INT sleepTime;
	};
}

#endif