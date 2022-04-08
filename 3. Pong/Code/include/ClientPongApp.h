#pragma once

#include "NetworkLibrary.h"
#include "SFML/Graphics.hpp"

#include "Player.h"
#include "Ball.h"

class ClientPongApp :    public ClientGame
{
public:
	virtual bool Initialize() override;

	virtual void Update(float deltaTime) override;

	virtual void Terminate() override;

	virtual bool IsRunning() override;
protected:
	virtual int HandleSinglePacket(char* data) override;

private:
	void PlayerInput();

	void Draw();

	Player* GetCurrentPlayer();

private:
	sf::RenderWindow m_Window;
	sf::Clock m_Clock;

	Ball* m_Ball;

	Player* m_PlayerLeft;
	Player* m_PlayerRight;

	uint32_t m_ScoreLeft;
	uint32_t m_ScoreRight;

	PlayerType m_PlayerType;
};

