#include "Player.h"

Player::Player()
{
}

Player::~Player() noexcept
{
	mciSendStringW(L"stop mp3", NULL, 0, NULL);
	mciSendStringW(L"close mp3", NULL, 0, NULL);
}

void Player::play() noexcept
{
	std::wstring cmd = L"open \"" + soundToPlay_.wstring() + L"\" type mpegvideo alias mp3";
	mciSendStringW(cmd.c_str(), NULL, 0, NULL);
	mciSendStringW(L"play mp3 repeat", NULL, 0, NULL);
}
