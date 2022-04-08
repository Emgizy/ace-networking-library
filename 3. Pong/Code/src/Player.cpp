#include "Player.h"

Player::Player()
{
}

Player::Player(const sf::Vector2f& pos)
{
	m_Rectangle.setPosition(pos);
	m_Rectangle.setFillColor(sf::Color::White);

	const float height = 50.0f;
	const float width = 12.0f;
	m_Rectangle.setSize({width, height});
	m_Rectangle.setOrigin({ width / 2.0f, height / 2.0f });

	m_Speed = 0.0f;
}

void Player::Update(float deltaTime)
{
	sf::Vector2f pos = m_Rectangle.getPosition();

	pos.y += deltaTime * m_Speed;

	m_Rectangle.setPosition(pos);
}

void Player::Draw(sf::RenderWindow& window) const
{
	window.draw(m_Rectangle);
}

void Player::KeepInsideBounds(const sf::RenderWindow& window)
{
	sf::Vector2f rect_pos = m_Rectangle.getPosition();
	float half_height = GetHeight() / 2.0f;
	sf::Vector2u window_size = window.getSize();

	if (rect_pos.y + half_height > window_size.y)
		rect_pos.y = window_size.y - half_height;

	if (rect_pos.y - half_height < 100.0f)
		rect_pos.y = 100.0f + half_height;

	m_Rectangle.setPosition(rect_pos);
}

sf::Vector2f Player::GetPosition() const
{
	return m_Rectangle.getPosition();
}

float Player::GetHeight() const
{
	return m_Rectangle.getSize().y;
}

float Player::GetWidth() const
{
	return m_Rectangle.getSize().x;
}

void Player::SetPosition(const sf::Vector2f pos)
{
	m_Rectangle.setPosition(pos);
}

void Player::SetVelocity(float speed)
{
	m_Speed = speed;
}

void Player::SetSelected(bool selection)
{
	if (selection)
	{
		m_Rectangle.setFillColor(sf::Color::Green);
	}
	else
	{
		m_Rectangle.setFillColor(sf::Color::White);
	}
}
