#include "includes.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

class Player
{
public:
	Player();
	~Player() noexcept;

	void play() noexcept;
private:
	const std::filesystem::path soundToPlay_ = L"A:\\CPP_projects\\CPP\\in place out of place\\assets\\sounds\\socialCreditSiren.mp3";
};