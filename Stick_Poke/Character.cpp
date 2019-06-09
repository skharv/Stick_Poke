#include "Character.h"

sf::Vector2f Character::CalculateElbow(sf::Vector2f Shoulder, sf::Vector2f Wrist)
{
	Shoulder = Shoulder - m_Position;
	Wrist = Wrist - m_Position;

	float ArmSectionLength = ARMSECTIONLENGTH;
	sf::Vector2f midSection = sf::Vector2f((Wrist.x + Shoulder.x) / 2, (Wrist.y + Shoulder.y) / 2);

	float midLength = Utility::GetMagnitude(midSection - Shoulder);

	float elbowDistance = sqrt(fabs((ArmSectionLength * ArmSectionLength) - (midLength * midLength)));
	sf::Vector2f elbow = Utility::Normalise(Wrist - Shoulder);

	elbow *= elbowDistance;

	sf::Vector2f elbowPos;

	elbowPos.x = -elbow.y;
	elbowPos.y = elbow.x;

	elbowPos += midSection;

	return elbowPos + m_Position;
}

void Character::UpdateArmPositions()
{
	m_BackShoulder = sf::Vector2f(m_Rect.getPosition().x - (m_Rect.getSize().x / 2), m_Rect.getPosition().y - (m_Rect.getSize().y / 2) + (m_Rect.getSize().y / 5));
	m_FrontShoulder = sf::Vector2f(m_Rect.getPosition().x + (m_Rect.getSize().x / 2), m_Rect.getPosition().y - (m_Rect.getSize().y / 2) + (m_Rect.getSize().y / 5));
	m_BackElbow = CalculateElbow(m_BackShoulder, m_Weapon->GetBackHandPos());
	m_FrontElbow = CalculateElbow(m_FrontShoulder, m_Weapon->GetFrontHandPos());

	m_BackBicep.setPosition(Utility::MidPointOfTwoVectors(m_BackShoulder, m_BackElbow));
	m_BackBicep.setRotation((Utility::AngleBetweenTwoVectors(m_BackShoulder, m_BackElbow) * 180 / ut::PI) + 90);

	m_BackElbowCircle.setPosition(m_BackElbow);

	m_BackForearm.setPosition(Utility::MidPointOfTwoVectors(m_BackElbow, m_Weapon->GetBackHandPos()));
	m_BackForearm.setRotation((Utility::AngleBetweenTwoVectors(m_BackElbow, m_Weapon->GetBackHandPos()) * 180 / ut::PI) + 90);

	m_FrontBicep.setPosition(Utility::MidPointOfTwoVectors(m_FrontShoulder, m_FrontElbow));
	m_FrontBicep.setRotation((Utility::AngleBetweenTwoVectors(m_FrontElbow, m_FrontShoulder) * 180 / ut::PI) + 90);

	m_FrontElbowCircle.setPosition(m_FrontElbow);

	m_FrontForearm.setPosition(Utility::MidPointOfTwoVectors(m_FrontElbow, m_Weapon->GetFrontHandPos()));
	m_FrontForearm.setRotation((Utility::AngleBetweenTwoVectors(m_FrontElbow, m_Weapon->GetFrontHandPos()) * 180 / ut::PI) + 90);
}

void Character::Draw(sf::RenderWindow &Window)
{
	Window.draw(m_FrontForearm);
	Window.draw(m_FrontElbowCircle);
	Window.draw(m_FrontBicep);
	Window.draw(m_Rect);
	Window.draw(m_BackBicep);
	Window.draw(m_BackElbowCircle);
	Window.draw(m_BackForearm);
	m_Weapon->Draw(Window, true);
}

void Character::Update(sf::Vector2f Gravity)
{
	b2Vec2 worldCenter = m_Body->GetWorldCenter();

	//------ MOVEMENT ------//
	m_Velocity = Utility::B2VECtoSFVEC(m_Body->GetLinearVelocity(), true);
	float xForce = 0;
	float yForce = 0;
	bool Impulse = false;

	switch (m_MoveState)
	{
	case MS_IDLE:
		if(m_State == STANDING)
			xForce = m_Velocity.x * -2;
		break;
	case MS_WALKLEFT:
		if (m_Velocity.x > -100)
			xForce = -100;
		else if(m_Velocity.x < -100 && m_State == STANDING)
			xForce = m_Velocity.x * -2;
		break;
	case MS_WALKRIGHT:
		if (m_Velocity.x < 100)
			xForce = 100;
		else if (m_Velocity.x > 100 && m_State == STANDING)
			xForce = m_Velocity.x * -2;
		break;
	case MS_DASHLEFT:
		xForce = -200;
		Impulse = true;
		break;
	case MS_DASHRIGHT:
		xForce = 200;
		Impulse = true;
		break;
	case MS_JUMPLEFT:
		yForce = -200;
		xForce = -100;
		Impulse = true;
		break;
	case MS_JUMPRIGHT:
		yForce = -200;
		xForce = 100;
		Impulse = true;
		break;
	case MS_JUMPUP:
		yForce = -200;
		Impulse = true;
		break;
	}

	if(Impulse)
		m_Body->ApplyLinearImpulse(Utility::SFVECtoB2VEC(sf::Vector2f(xForce, yForce)), worldCenter, true);
	else
		m_Body->ApplyForce(Utility::SFVECtoB2VEC(sf::Vector2f(xForce, yForce)), worldCenter, true);

	m_Position = sf::Vector2f(Utility::B2VECtoSFVEC(m_Body->GetPosition(), true));
	m_Rect.setPosition(m_Position);
	m_Weapon->Update(m_WeaponPos, m_Position, m_Rect.getSize(), false, false);
	UpdateArmPositions();
}

void Character::ReceiveInputs(Input* State)
{
	m_WeaponPos = State->GetInputBuffer()->rStick;

	if (State->GetInputBuffer()->lStick.x < -20 || State->GetInputBuffer()->left)
	{
		if (!State->WasInputPressedAtFrame(Input::c_Movement, 1))
			m_MoveState = MS_WALKLEFT;
		else
			if (m_State == STANDING)
				m_MoveState = MS_DASHLEFT;

		if((State->GetInputBuffer()->lStick.y < -20 || State->GetInputBuffer()->up)&& State->WasInputPressedAtFrame(Input::c_Movement, 1))
			if(m_State != AIRBORNE)
				m_MoveState = MS_JUMPLEFT;
	}
	else if (State->GetInputBuffer()->lStick.x > 20 || State->GetInputBuffer()->right)
	{
		if (!State->WasInputPressedAtFrame(Input::c_Movement, 1))
			m_MoveState = MS_WALKRIGHT;
		else
			if(m_State == STANDING)
				m_MoveState = MS_DASHRIGHT;

		if ((State->GetInputBuffer()->lStick.y < -20 || State->GetInputBuffer()->up) && State->WasInputPressedAtFrame(Input::c_Movement, 1))
			if (m_State != AIRBORNE)
				m_MoveState = MS_JUMPRIGHT;
	}
	else
	{
		m_MoveState = MS_IDLE;

		if ((State->GetInputBuffer()->lStick.y < -20 || State->GetInputBuffer()->up) && State->WasInputPressedAtFrame(Input::c_Movement, 1))
			if (m_State != AIRBORNE)
				m_MoveState = MS_JUMPUP;
	}
}

Character::Character(b2World &World)
{
	m_Rect.setSize(sf::Vector2f(50.0f, 100.0f));
	m_Rect.setOrigin(m_Rect.getSize().x / 2, m_Rect.getSize().y / 2);
	m_Rect.setPosition(0, 0);
	m_Rect.setFillColor(sf::Color::Yellow);

	m_BackBicep.setSize(sf::Vector2f(10.0f, ARMSECTIONLENGTH));
	m_BackBicep.setOrigin(m_BackBicep.getSize().x / 2, m_BackBicep.getSize().y / 2);
	m_BackBicep.setPosition(0, 0);
	m_BackBicep.setFillColor(sf::Color::Magenta);

	m_BackForearm.setSize(sf::Vector2f(10.0f, ARMSECTIONLENGTH));
	m_BackForearm.setOrigin(m_BackForearm.getSize().x / 2, m_BackForearm.getSize().y / 2);
	m_BackForearm.setPosition(0, 0);
	m_BackForearm.setFillColor(sf::Color::Magenta);

	m_FrontBicep.setSize(sf::Vector2f(10.0f, ARMSECTIONLENGTH));
	m_FrontBicep.setOrigin(m_FrontBicep.getSize().x / 2, m_FrontBicep.getSize().y / 2);
	m_FrontBicep.setPosition(0, 0);
	m_FrontBicep.setFillColor(sf::Color::Cyan);

	m_FrontForearm.setSize(sf::Vector2f(10.0f, ARMSECTIONLENGTH));
	m_FrontForearm.setOrigin(m_FrontForearm.getSize().x / 2, m_FrontForearm.getSize().y / 2);
	m_FrontForearm.setPosition(0, 0);
	m_FrontForearm.setFillColor(sf::Color::Cyan);

	m_BackElbowCircle.setRadius(5);
	m_BackElbowCircle.setOrigin(5, 5);
	m_BackElbowCircle.setFillColor(sf::Color::Magenta);
	m_BackElbowCircle.setOutlineColor(sf::Color(0, 0, 0, 0));
	m_BackElbowCircle.setPosition(0, 0);

	m_FrontElbowCircle.setRadius(5);
	m_FrontElbowCircle.setOrigin(5, 5);
	m_FrontElbowCircle.setFillColor(sf::Color::Cyan);
	m_FrontElbowCircle.setOutlineColor(sf::Color(0, 0, 0, 0));
	m_FrontElbowCircle.setPosition(0, 0);

	m_Weapon = new Weapon(World);
	m_Weapon->SetAnchorOffset(sf::Vector2f(-75.0, 15.0f));
	m_Weapon->SetControlRadius(60.0f);
	m_Weapon->SetControlOffset(sf::Vector2f(110.0f, 0.0f));
	m_Weapon->SetWeaponSize(sf::Vector2f(180.0, 10.0f));
	m_Weapon->SetWeaponOrigin(sf::Vector2f(160.0, 5.0f));
	m_Weapon->SetHitboxOffset(sf::Vector2f(15.0, 0.0f));
	m_Weapon->SetHitboxRadius(7.5f);

	for (int i = 0; i < m_BackArm.getVertexCount(); i++)
	{
		m_BackArm[i].color = sf::Color::Magenta;
	}

	for (int i = 0; i < m_BackArm.getVertexCount(); i++)
	{
		m_FrontArm[i].color = sf::Color::Cyan;
	}

	m_WeaponPos = sf::Vector2f(0, 0);

	InitRoundedBody(World, m_Rect.getSize());

	m_State = IDLE;
	m_Body->GetFixtureList()[0].SetDensity(100);
	
	b2PolygonShape polygonShape;
	polygonShape.SetAsBox(Utility::ScaleToB2(m_Rect.getSize().x / 2.5), Utility::ScaleToB2(m_Rect.getSize().y / 20), Utility::SFVECtoB2VEC(sf::Vector2f(0, m_Rect.getSize().y / 2), true), 0);
	b2FixtureDef myFixtureDef;
	myFixtureDef.isSensor = true;
	myFixtureDef.shape = &polygonShape;
	b2Fixture* footSensorFixture = m_Body->CreateFixture(&myFixtureDef);
}

Character::Character()
{
}

Character::~Character()
{
}
