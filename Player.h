#include "includes.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

class Player
{
public:
	Player();
	~Player();

	void play();
	void stop();

private:
	const std::wstring soundToPlay = L"A:\\CPP_projects\\CPP\\in place out of place\\assets\\sounds\\socialCreditSiren.mp3";
};