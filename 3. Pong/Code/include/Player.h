#pragma once

#include "SFML/Graphics.hpp"

#include "NetworkLibrary.h"

enum class PlayerType
{
	LEFT,
	RIGHT,
	NONE
};

class PlayerPositionPacket : public Packet
{
public:
	PlayerPositionPacket() : Packet(PacketType::PLAYER_POSITION) { m_PlayerType = PlayerType::NONE; }
	sf::Vector2f m_Position;
	PlayerType m_PlayerType;

	virtual void Serialize(char* data) override {
		memcpy(data, &m_Type, sizeof(PacketType)); //first serialize the type

		memcpy(data + sizeof(PacketType), &m_Position, sizeof(sf::Vector2f)); //then serialize vector

		memcpy(data + sizeof(PacketType) + sizeof(sf::Vector2f), &m_PlayerType, sizeof(PlayerType)); //then serialize player type
	}

	virtual void Deserialize(char* data) override {
		memcpy(&m_Type, data, sizeof(PacketType)); //first deserialize the type

		memcpy(&m_Position, data + sizeof(PacketType), sizeof(sf::Vector2f)); //then deserialize the vector

		memcpy(&m_PlayerType, data + sizeof(PacketType) + sizeof(sf::Vector2f), sizeof(PlayerType)); //then deserialize the player type
	}

	virtual int SizeOfPacket() const override {
		return sizeof(PacketType) + sizeof(sf::Vector2f) + sizeof(PlayerType);
	}
};

class PlayerVelocityPacket : public Packet
{
public:
	PlayerVelocityPacket() : Packet(PacketType::PLAYER_VELOCITY) { m_PlayerType = PlayerType::NONE; m_Velocity = 0.0f; }
	float m_Velocity;
	PlayerType m_PlayerType;

	virtual void Serialize(char* data) override {
		memcpy(data, &m_Type, sizeof(PacketType)); //first serialize the type

		memcpy(data + sizeof(PacketType), &m_Velocity, sizeof(float)); //then serialize velocity

		memcpy(data + sizeof(PacketType) + sizeof(float), &m_PlayerType, sizeof(PlayerType)); //then serialize player type
	}

	virtual void Deserialize(char* data) override {
		memcpy(&m_Type, data, sizeof(PacketType)); //first deserialize the type

		memcpy(&m_Velocity, data + sizeof(PacketType), sizeof(float)); //then deserialize the velocity

		memcpy(&m_PlayerType, data + sizeof(PacketType) + sizeof(float), sizeof(PlayerType)); //then deserialize the player type
	}

	virtual int SizeOfPacket() const override {
		return sizeof(PacketType) + sizeof(float) + sizeof(PlayerType);
	}
};

class SetPlayerControlPacket : public Packet
{
public:
	SetPlayerControlPacket() : Packet(PacketType::PLAYER_CONTROL) { m_PlayerType = PlayerType::NONE; }

	PlayerType m_PlayerType;
	virtual void Serialize(char* data) override {
		memcpy(data, &m_Type, sizeof(PacketType)); //first serialize the type

		memcpy(data + sizeof(PacketType), &m_PlayerType, sizeof(PlayerType)); //then serialize player type
	}

	virtual void Deserialize(char* data) override {
		memcpy(&m_Type, data, sizeof(PacketType)); //first deserialize the type

		memcpy(&m_PlayerType, data + sizeof(PacketType), sizeof(PlayerType)); //then deserialize the player type
	}

	virtual int SizeOfPacket() const override {
		return sizeof(PacketType) + sizeof(PlayerType);
	}
};

class SetScorePacket : public Packet
{
public:
	SetScorePacket() : Packet(PacketType::SCORE) { m_PlayerType = PlayerType::NONE; m_Score = 0; }
	uint32_t m_Score;
	PlayerType m_PlayerType;

	virtual void Serialize(char* data) override {
		memcpy(data, &m_Type, sizeof(PacketType)); //first serialize the type

		memcpy(data + sizeof(PacketType), &m_Score, sizeof(uint32_t)); //then serialize score

		memcpy(data + sizeof(PacketType) + sizeof(uint32_t), &m_PlayerType, sizeof(PlayerType)); //then serialize player type
	}

	virtual void Deserialize(char* data) override {
		memcpy(&m_Type, data, sizeof(PacketType)); //first deserialize the type

		memcpy(&m_Score, data + sizeof(PacketType), sizeof(uint32_t)); //then deserialize the score

		memcpy(&m_PlayerType, data + sizeof(PacketType) + sizeof(uint32_t), sizeof(PlayerType)); //then deserialize the player type
	}

	virtual int SizeOfPacket() const override {
		return sizeof(PacketType) + sizeof(uint32_t) + sizeof(PlayerType);
	}
};

class Player
{
public:
	Player();
	Player(const sf::Vector2f& pos);


	void Update(float deltaTime);

	void Draw(sf::RenderWindow& window) const;

	void KeepInsideBounds(const sf::RenderWindow& window);

	// getter
	sf::Vector2f GetPosition() const;
	float GetHeight() const;
	float GetWidth() const;

	// setter
	void SetPosition(const sf::Vector2f pos);

	void SetVelocity(float speed);

	void SetSelected(bool selection);

private:
	sf::RectangleShape m_Rectangle;

	float m_Speed;
};