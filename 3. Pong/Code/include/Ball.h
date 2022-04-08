#pragma once

#include "SFML/Graphics.hpp"

#include "NetworkLibrary.h"

#include "Player.h"

class BallPositionPacket : public Packet
{
public:
	BallPositionPacket() : Packet(PacketType::BALL_POSITION) {}
	sf::Vector2f m_Position;

	virtual void Serialize(char* data) override {
		memcpy(data, &m_Type, sizeof(PacketType)); //first serialize the type

		memcpy(data + sizeof(PacketType), &m_Position, sizeof(sf::Vector2f)); //then serialize vector
	}

	virtual void Deserialize(char* data) override {
		memcpy(&m_Type, data, sizeof(PacketType)); //first deserialize the type

		memcpy(&m_Position, data + sizeof(PacketType), sizeof(sf::Vector2f)); //then deserialize the vector
	}

	virtual int SizeOfPacket() const override {
		return sizeof(PacketType) + sizeof(sf::Vector2f);
	}
};

class Ball
{
public:
	Ball();
	Ball(const sf::Vector2f& pos);

	void Update(float deltaTime);

	void Draw(sf::RenderWindow& window) const;

	void KeepInsideBounds(const sf::RenderWindow& window);

	// getter
	sf::Vector2f GetPosition() const;

	sf::Vector2f GetVelocity() const;

	// setter
	void SetPosition(const sf::Vector2f pos);

	void SetVelocity(const sf::Vector2f vel);

	// utils
	bool Intersects(Player* player);

private:
	sf::CircleShape m_Circle;
	sf::Vector2f m_Velocity;
};