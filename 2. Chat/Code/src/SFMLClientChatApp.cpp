#include "SFMLClientChatApp.h"
#include "NetworkLibrary.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"

#include <chrono>

bool SFMLClientChatApp::Initialize()
{
    if (m_Client.connect(DEFAULT_HOST, atoi(DEFAULT_PORT)) != sf::Socket::Done)
    {
        // error...
        return false;
    }
    m_Client.setBlocking(false);

    m_Window.create(sf::VideoMode(800, 600), "Technical Computing Project - SFML Client Chat - Marc Garcia Carda");
    ImGui::SFML::Init(m_Window);

    m_Clock.restart();

    return true;
}

void SFMLClientChatApp::Update(float deltaTime)
{
    sf::Time dt = m_Clock.restart();
    m_DeltaTime = dt.asSeconds();
    HandleData();

    // gameplay code
    sf::Event event;
    while (m_Window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed)
            m_Window.close();
    }

    ImGui::SFML::Update(m_Window, dt);

    DrawImGui();


#if TESTING
    Testing();
#endif

    sf::Color col = sf::Color(133, 192, 214);
    m_Window.clear(col);
    ImGui::SFML::Render(m_Window);
    m_Window.display();
}

void SFMLClientChatApp::Terminate()
{
    ImGui::SFML::Shutdown();
    m_Window.close();
}

bool SFMLClientChatApp::IsRunning()
{
    return m_Window.isOpen();
}

void SFMLClientChatApp::DrawImGui()
{
    ImGuiConsole();
    ImGuiStats();
}

void SFMLClientChatApp::ImGuiConsole()
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
            sf::Packet msm_packet;
            msm_packet << input_str;
            m_Client.send(msm_packet);

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

void SFMLClientChatApp::ImGuiStats()
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
    ImGui::Text(std::to_string(m_TotalNumberOfPacketsSent).c_str());
    ImGui::SameLine();
    ImGui::Text("Time in Microseconds: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(m_TotalTimeSendingPackets).c_str());

    ImGui::Text("Packets Sent Per Microsecond: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(m_TotalNumberOfPacketsSent / (float)m_TotalTimeSendingPackets).c_str());

    ImGui::Text("Total Packets Received: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(m_TotalNumberOfPacketsReceived).c_str());
    ImGui::SameLine();
    ImGui::Text("Time in Microseconds: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(m_TotalTimeReceivingPackets).c_str());

    ImGui::Text("Packets Received Per Microsecond: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(m_TotalNumberOfPacketsReceived / (float)m_TotalTimeReceivingPackets).c_str());
#endif


    ImGui::End();
}

void SFMLClientChatApp::AddChatMessage(const std::string& str)
{
    m_TextBuffer.append(str.c_str());
    m_TextBuffer.append("\n");
}

void SFMLClientChatApp::HandleData()
{
#if TESTING
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    sf::Packet packet;

    if (m_Client.receive(packet) != sf::Socket::Done)
    {
        // error...
        return;
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    m_TotalTimeReceivingPackets += duration;

    std::string str;
    if (packet >> str) // if correct deserialization of packet
    {
        AddChatMessage(str);
        ++m_TotalNumberOfPacketsReceived;
    }
#else

    sf::Packet packet;
    if (m_Client.receive(packet) != sf::Socket::Done)
    {
        // error...
        return;
    }
    std::string str;
    if (packet >> str) // if correct deserialization of packet
    {
        AddChatMessage(str);
    }
#endif



}

void SFMLClientChatApp::Testing()
{
    sf::Packet packet;
    std::string str = "Hello world!";
    packet << str;
    const int packets_sent = 50;
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < packets_sent; ++i)
    {
        if (m_Client.send(packet) == sf::Socket::Done)
        {
            ++m_TotalNumberOfPacketsSent;
        }
        else {
            break;
        }
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    m_TotalTimeSendingPackets += duration;

    m_TextBuffer.clear();
}
