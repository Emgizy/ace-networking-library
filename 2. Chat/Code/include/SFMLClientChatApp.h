#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/Network.hpp"
#include "imgui.h"

class SFMLClientChatApp
{
public:
	bool Initialize();

	void Update(float deltaTime);

	void Terminate();

	bool IsRunning();


private:
	// Imgui
	void DrawImGui();
	void ImGuiConsole();
	void ImGuiStats();

	// Chat
	void AddChatMessage(const std::string& str);

	// client
	void HandleData();

	// testing
	void Testing();

private:
	sf::RenderWindow m_Window;
	sf::Clock m_Clock;

	// imgui chat
	ImGuiTextBuffer     m_TextBuffer;

	// SFML networking
	sf::TcpSocket m_Client;

	// stats
	float m_DeltaTime;

	uint32_t m_TotalNumberOfPacketsSent = 0;
	uint32_t m_TotalNumberOfPacketsReceived = 0;
	long long m_TotalTimeSendingPackets = 0; //microsecond
	long long m_TotalTimeReceivingPackets = 0; //microsecond
};

