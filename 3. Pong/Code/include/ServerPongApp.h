#pragma once

#include "NetworkLibrary.h"
#include "SFML/Graphics.hpp"

#include "Player.h"
#include "Ball.h"

#include <deque>

class ServerPongApp : public ServerGame
{
public:
	virtual bool Initialize() override;

	virtual void Update(float deltaTime) override;

	virtual void Terminate() override;

	virtual bool IsRunning() override;

protected:
	virtual int HandleSinglePacket(uint32_t client_id, char* data) override;


private:
	void Draw();
	void DrawImGui();

	void ResetGame();

private:
	sf::RenderWindow m_Window;
	sf::Clock m_Clock;

	Ball* m_Ball;

	Player* m_PlayerLeft;
	Player* m_PlayerRight;

	int32_t m_ClientIdLeft;
	int32_t m_ClientIdRight;

	uint32_t m_ScoreLeft;
	uint32_t m_ScoreRight;

	std::deque<uint32_t> m_WaitQueue;
};

