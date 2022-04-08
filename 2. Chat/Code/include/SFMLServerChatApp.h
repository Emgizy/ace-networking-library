#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/Network.hpp"
#include "imgui.h"

class SFMLServerChatApp
{
public:
	bool Initialize();

	void Update(float deltaTime);

	void Terminate();

	bool IsRunning();

protected:
	// server
	void AcceptClients();
	void HandleData();

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

	// SFML Network
	sf::TcpListener m_Listener;
	std::map<uint32_t, sf::TcpSocket> m_ClientSessions;


	// stats
	float m_DeltaTime;


	uint32_t m_TotalNumberOfPacketsSent = 0;
	uint32_t m_TotalNumberOfPacketsReceived = 0;
	long long m_TotalTimeSendingPackets = 0; //microsecond
	long long m_TotalTimeReceivingPackets = 0; //microsecond
};

