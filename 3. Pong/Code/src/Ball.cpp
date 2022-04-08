#include "Ball.h"



Ball::Ball()
{
}

Ball::Ball(const sf::Vector2f& pos) :
	m_Velocity(-100.0f, -100.0f)
{
	m_Circle.setPosition(pos);
	m_Circle.setFillColor(sf::Color::White);

	const float radius = 6.0f;
	m_Circle.setRadius(radius);
	m_Circle.setOrigin({ radius, radius});
}

void Ball::Update(float deltaTime)
{
	sf::Vector2f pos = m_Circle.getPosition();
	pos += m_Velocity * deltaTime;
	m_Circle.setPosition(pos);
}

void Ball::Draw(sf::RenderWindow& window) const
{
	window.draw(m_Circle);
}

void Ball::KeepInsideBounds(const sf::RenderWindow& window)
{
	sf::Vector2f circle_pos = m_Circle.getPosition();
	float circle_rad = m_Circle.getRadius();
	sf::Vector2u window_size = window.getSize();

	//if (circle_pos.x + circle_rad > window_size.x || circle_pos.x - circle_rad < 0.0f)
	//	m_Velocity.x *= -1;

	if (circle_pos.y + circle_rad > window_size.y)
	{
		circle_pos.y = window_size.y - circle_rad;
		m_Circle.setPosition(circle_pos);

		m_Velocity.y = -std::abs(m_Velocity.y);
	}

	if (circle_pos.y - circle_rad < 100.0f)
	{
		circle_pos.y =  100.0f + circle_rad;
		m_Circle.setPosition(circle_pos);

		m_Velocity.y = std::abs(m_Velocity.y);
	}
}

sf::Vector2f Ball::GetPosition() const
{
	return m_Circle.getPosition();
}

sf::Vector2f Ball::GetVelocity() const
{
	return m_Velocity;
}

void Ball::SetPosition(const sf::Vector2f pos)
{
	m_Circle.setPosition(pos);
}

void Ball::SetVelocity(const sf::Vector2f vel)
{
	m_Velocity = vel;
}

bool Ball::Intersects(Player* player)
{
	//based on https://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection
	if (!player) return false;
	sf::Vector2f circle_distance;
	circle_distance.x = std::abs(m_Circle.getPosition().x - player->GetPosition().x);
	circle_distance.y = std::abs(m_Circle.getPosition().y - player->GetPosition().y);


	if (circle_distance.x > (player->GetWidth() / 2.0f + m_Circle.getRadius())) { return false; }
	if (circle_distance.y > (player->GetHeight() / 2.0f + m_Circle.getRadius())) { return false; }

	if (circle_distance.x <= (player->GetWidth() / 2.0f)) { return true; }
	if (circle_distance.y <= (player->GetHeight() / 2.0f)) { return true; }

	float corner_distance_sq = std::pow(circle_distance.x - player->GetWidth() / 2.0f, 2.0f) +
		std::pow(circle_distance.y - player->GetHeight() / 2.0f,  2.0f);

	return (corner_distance_sq <= std::pow(m_Circle.getRadius(), 2.0f));
}
