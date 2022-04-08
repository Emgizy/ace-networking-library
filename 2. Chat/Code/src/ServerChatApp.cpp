#include "ServerChatApp.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"

#include <chrono>

uint32_t g_TotalNumberOfPacketsSent = 0;
uint32_t g_TotalNumberOfPacketsReceived = 0;
long long g_TotalTimeSendingPackets = 0; //microsecond
long long g_TotalTimeReceivingPackets = 0; //microsecond


bool ServerChatApp::Initialize()
{
	bool init = ServerGame::Initialize();

	if (!init)
	{
		std::cout << "Server not initialized correctly." << std::endl;
		return false;
	}

	std::cout << "Server initialized correctly." << std::endl;
	m_Window.create(sf::VideoMode(800, 600), "Technical Computing Project - My Server Chat - Marc Garcia Carda");
	ImGui::SFML::Init(m_Window);

	m_Clock.restart();

	return true;
}

void ServerChatApp::Update(float deltaTime)
{
	sf::Time dt = m_Clock.restart();
	m_DeltaTime = dt.asSeconds();
	ServerGame::Update(m_DeltaTime);

	// gameplay code
	sf::Event event;
	while (m_Window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
			m_Window.close();
	}

	ImGui::SFML::Update(m_Window, dt);

#if TESTING
	m_TextBuffer.clear();
#endif


	DrawImGui();

	sf::Color col = sf::Color(151, 209, 137);
	m_Window.clear(col);
	ImGui::SFML::Render(m_Window);
	m_Window.display();
}

void ServerChatApp::Terminate()
{
	// send an terminate connection packet to all clients
	Packet packet;
	packet.m_Type = PacketType::TERMINATE_CONNECTION;
	m_Server->SendPacketToAllClients(&packet);

	ImGui::SFML::Shutdown();
	m_Window.close();
}

bool ServerChatApp::IsRunning()
{
	return ServerGame::IsRunning() && m_Window.isOpen();
}


int ServerChatApp::HandleSinglePacket(uint32_t client_id, char* data)
{
	Packet packet;
	//find packet type
	packet.Deserialize(data);

	int size = packet.SizeOfPacket();
	switch (packet.m_Type)
	{
	case PacketType::INITIALIZE_CONNECTION:
	{
		std::string message = "[SERVER] Client " + std::to_string(client_id);
		message += " has connected to the server.";

		MessagePacket msg_packet;
		msg_packet.m_Message = message;

		m_Server->SendPacketToAllClients(&msg_packet);
		AddChatMessage(message);
	}
	break;
	case PacketType::TERMINATE_CONNECTION:
	{
		std::string message = "[SERVER] Client " + std::to_string(client_id);
		message += " has disconected from the server.";

		MessagePacket msg_packet;
		msg_packet.m_Message = message;

		m_Server->SendPacketToAllClients(&msg_packet);
		AddChatMessage(message);

		m_Server->RemoveClient(client_id);
	}
	break;
	case PacketType::MESSAGE:
	{
		// read original message
		MessagePacket msg_packet;
		msg_packet.Deserialize(data);
		size = msg_packet.SizeOfPacket();

		// create message to display
		std::string message = "[Client " + std::to_string(client_id);
		message += "] " + msg_packet.m_Message;

		// send new packet to all clients
		MessagePacket new_msg_packet;
		new_msg_packet.m_Message = message;

		m_Server->SendPacketToAllClients(&new_msg_packet);
		AddChatMessage(new_msg_packet.m_Message);
	}
	break;
	default:
		break;
	}

	return size;
}



void ServerChatApp::DrawImGui()
{
	ImGuiConnections();
	ImGuiConsole();
	ImGuiStats();
}


void ServerChatApp::ImGuiConsole()
{
	if (ImGui::Begin("Server Console"))
	{
		if (ImGui::Button("Clear"))
		{
			m_TextBuffer.clear();
		}
		ImGui::SameLine();

		static std::string input_str;
		if (ImGui::InputText("##input", &input_str, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			input_str = "[SERVER] " + input_str;
			AddChatMessage(input_str);

			MessagePacket msg_packet;
			msg_packet.m_Message = input_str;
			m_Server->SendPacketToAllClients(&msg_packet);

			input_str.clear();
			ImGui::SetKeyboardFocusHere();
		}

		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImGui::TextUnformatted(m_TextBuffer.begin(), m_TextBuffer.end());

			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);
		}


		ImGui::EndChild();
	}

	ImGui::End();
}

void ServerChatApp::ImGuiConnections()
{
	ImGui::Begin("Connections");
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnOffset(1, 80);
	ImGui::Text("Client ID");
	ImGui::NextColumn();
	ImGui::Text("SOCKET");
	ImGui::NextColumn();

	for (const auto& client : m_Server->m_ClientSessions)
	{
		ImGui::Text(std::to_string(client.first).c_str());
		ImGui::NextColumn();
		ImGui::Text(std::to_string(client.second).c_str());
		ImGui::NextColumn();
	}
	ImGui::End();

}

void ServerChatApp::ImGuiStats()
{
	ImGui::Begin("Stats");

	ImGui::Text("DeltaTime: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(m_DeltaTime).c_str());


	float fps = 1.0f / m_DeltaTime;
	ImGui::Text("FPS: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(fps).c_str());

#if TESTING
	ImGui::Text("Total Packets Sent: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(g_TotalNumberOfPacketsSent).c_str());
	ImGui::SameLine();
	ImGui::Text("Time in Microseconds: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(g_TotalTimeSendingPackets).c_str());

	ImGui::Text("Packets Sent Per Microsecond: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(g_TotalNumberOfPacketsSent / (float)g_TotalTimeSendingPackets).c_str());

	ImGui::Text("Total Packets Received: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(g_TotalNumberOfPacketsReceived).c_str());
	ImGui::SameLine();
	ImGui::Text("Time in Microseconds: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(g_TotalTimeReceivingPackets).c_str());

	ImGui::Text("Packets Received Per Microsecond: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(g_TotalNumberOfPacketsReceived / (float)g_TotalTimeReceivingPackets).c_str());
#endif

	ImGui::End();
}

void ServerChatApp::AddChatMessage(const std::string& str)
{
	m_TextBuffer.append(str.c_str());
	m_TextBuffer.append("\n");
}
