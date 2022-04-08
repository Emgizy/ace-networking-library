#include "ClientChatApp.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"

uint32_t g_TotalNumberOfPacketsSent = 0;
uint32_t g_TotalNumberOfPacketsReceived = 0;
long long g_TotalTimeSendingPackets = 0; //microsecond
long long g_TotalTimeReceivingPackets = 0; //microsecond

bool ClientChatApp::Initialize()
{
	bool init = ClientGame::Initialize();
	if (!init)
	{
		std::cout << "Client not initialized correctly." << std::endl;
		return false;
	}

	std::cout << "Client initialized correctly." << std::endl;

	m_Window.create(sf::VideoMode(800, 600), "Technical Computing Project - My Client Chat - Marc Garcia Carda");
	ImGui::SFML::Init(m_Window);

	m_Clock.restart();

	// send an initialize connection packet to server
	Packet packet(PacketType::INITIALIZE_CONNECTION);

	int sending = m_Client->SendPacketToServer(&packet);

	return sending != SOCKET_ERROR;
}

void ClientChatApp::Update(float deltaTime)
{
	sf::Time dt = m_Clock.restart();
	m_DeltaTime = dt.asSeconds();
	ClientGame::Update(m_DeltaTime);

	// gameplay code
	sf::Event event;
	while (m_Window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
			m_Window.close();
	}

#if TESTING
	MessagePacket msg;
	msg.m_Message = "Hello World!";
	for (int i = 0; i < 1000; ++i)
	{
		m_Client->SendPacketToServer(&msg);
	}
	m_TextBuffer.clear();
#endif

	ImGui::SFML::Update(m_Window, dt);


	DrawImGui();

	sf::Color col = sf::Color(133, 192, 214);
	m_Window.clear(col);
	ImGui::SFML::Render(m_Window);
	m_Window.display();
}

void ClientChatApp::Terminate()
{
	// send an terminate connection packet to server
	Packet packet;
	packet.m_Type = PacketType::TERMINATE_CONNECTION;
	m_Client->SendPacketToServer(&packet);

	ImGui::SFML::Shutdown();
	m_Window.close();
}

bool ClientChatApp::IsRunning()
{
	return ClientGame::IsRunning() && m_Window.isOpen();
}

int ClientChatApp::HandleSinglePacket(char* data)
{
	Packet packet;

	packet.Deserialize(data);

	int size = packet.SizeOfPacket();
	switch (packet.m_Type)
	{
	case PacketType::INITIALIZE_CONNECTION:
	{
		std::string message = "Connected to server.";

		AddChatMessage(message);
	}
	break;
	case PacketType::TERMINATE_CONNECTION:
	{
		std::string message = "Disconnected from server.";

		AddChatMessage(message);
	}
	break;
	case PacketType::MESSAGE:
	{
		MessagePacket msg_packet;
		msg_packet.Deserialize(data);

		size = msg_packet.SizeOfPacket();
		AddChatMessage(msg_packet.m_Message);
	}
	break;
	default:
		break;
	}

	return size;
}

void ClientChatApp::DrawImGui()
{
	ImGuiConsole();
	ImGuiStats();
}

void ClientChatApp::ImGuiConsole()
{
	if (ImGui::Begin("Client Console"))
	{
		if (ImGui::Button("Clear"))
		{
			m_TextBuffer.clear();
		}
		ImGui::SameLine();

		static std::string input_str;
		if (ImGui::InputText("##input", &input_str,
			ImGuiInputTextFlags_EnterReturnsTrue))
		{
			MessagePacket msg_packet;
			msg_packet.m_Message = input_str;
			m_Client->SendPacketToServer(&msg_packet);

			input_str.clear();
			ImGui::SetKeyboardFocusHere();
		}
		ImGui::SameLine();
		if (ImGui::Button("Disconnect"))
		{
			Packet packet(PacketType::TERMINATE_CONNECTION);
			m_Client->SendPacketToServer(&packet);
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

void ClientChatApp::ImGuiStats()
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

void ClientChatApp::AddChatMessage(const std::string& str)
{
	m_TextBuffer.append(str.c_str());
	m_TextBuffer.append("\n");
}
