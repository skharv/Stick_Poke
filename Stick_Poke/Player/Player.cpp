#include "Player.h"

void Player::Initialize()
{
}

void Player::Update()
{
	if (m_Input->ReadInputs().right)
	{
	}
}

void Player::ProcessInputs()
{
	m_Input->TakeInputs(0);
}

Player::Player()
{
	m_Input = new Input();
	m_Input->SetBindings(4, 5, 6, 7, 8, 9, Joystick::Axis::X, Joystick::Axis::Y, Joystick::Axis::U, Joystick::Axis::V); //These values are just whatevs atm.
}

Player::~Player()
{
}
