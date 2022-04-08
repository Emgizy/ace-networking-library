#pragma once

#include "NetworkLibrary.h"
#include "SFML/Graphics.hpp"
#include "imgui.h"

class ServerChatApp :    public ServerGame
{
public:
	virtual bool Initialize() override;

	virtual void Update(float deltaTime) override;

	virtual void Terminate() override;

	virtual bool IsRunning() override;

protected:
	virtual int HandleSinglePacket(uint32_t client_id, char* data) override;


private:
	// Imgui
	void DrawImGui();
	void ImGuiConsole();
	void ImGuiConnections();
	void ImGuiStats();

	// Chat
	void AddChatMessage(const std::string& str);

private:
	sf::RenderWindow m_Window;
	sf::Clock m_Clock;

	// imgui chat
	ImGuiTextBuffer	m_TextBuffer;

	// stats
	float m_DeltaTime;
};

