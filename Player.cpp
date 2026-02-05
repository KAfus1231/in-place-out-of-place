#include "Player.h"

Player::~Player() noexcept
{
	mciSendStringW((L"stop " + alias_).c_str(), NULL, 0, NULL);
	mciSendStringW((L"close " + alias_).c_str(), NULL, 0, NULL);
}

void Player::play() noexcept
{
	std::wstring cmd = L"open \"" + soundToPlay_.wstring() + L"\" type mpegvideo alias " + alias_;
	mciSendStringW(cmd.c_str(), NULL, 0, NULL);
	mciSendStringW((L"play " + alias_ + L" repeat").c_str(), NULL, 0, NULL);
}
