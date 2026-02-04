#include "Player.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::play()
{
	std::wstring cmd = L"open \"" + soundToPlay + L"\" type mpegvideo alias mp3";
	mciSendStringW(cmd.c_str(), NULL, 0, NULL);
	mciSendStringW(L"play mp3 repeat", NULL, 0, NULL);
}

void Player::stop()
{
	mciSendStringW(L"stop mp3", NULL, 0, NULL);
	mciSendStringW(L"close mp3", NULL, 0, NULL);
}
