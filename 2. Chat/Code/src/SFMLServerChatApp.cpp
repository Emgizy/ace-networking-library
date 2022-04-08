#include "SFMLServerChatApp.h"
#include "NetworkLibrary.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"

#include <chrono>

bool SFMLServerChatApp::Initialize()
{
    m_Listener.setBlocking(false);
    // bind the listener to a port
    if (m_Listener.listen(atoi(DEFAULT_PORT)) != sf::Socket::Done)
    {
        return false;
    }

    m_Window.create(sf::VideoMode(800, 600), "Technical Computing Project - SFML Server Chat - Marc Garcia Carda");
    ImGui::SFML::Init(m_Window);

    m_Clock.restart();

    return true;
}

void SFMLServerChatApp::Update(float)
{
    sf::Time dt = m_Clock.restart();
    m_DeltaTime = dt.asSeconds();
    AcceptClients();
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
    m_TextBuffer.clear();
#endif


    sf::Color col = sf::Color(151, 209, 137);
    m_Window.clear(col);
    ImGui::SFML::Render(m_Window);
    m_Window.display();
}

void SFMLServerChatApp::Terminate()
{
    ImGui::SFML::Shutdown();
    m_Window.close();
}

bool SFMLServerChatApp::IsRunning()
{
    return m_Window.isOpen();
}

void SFMLServerChatApp::DrawImGui()
{
    ImGuiConnections();
    ImGuiConsole();
    ImGuiStats();
}

void SFMLServerChatApp::ImGuiConsole()
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

            sf::Packet msm_packet;
            msm_packet << input_str;
            for (auto& client : m_ClientSessions)
            {
                client.second.send(msm_packet);
            }

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

void SFMLServerChatApp::ImGuiConnections()
{
    ImGui::Begin("Connections");
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 80);
    ImGui::Text("Client ID");
    ImGui::NextColumn();
    ImGui::Text("SOCKET");
    ImGui::NextColumn();

    for (const auto& client : m_ClientSessions)
    {
        ImGui::Text(std::to_string(client.first).c_str());
        ImGui::NextColumn();
        ImGui::Text(std::to_string(client.second.getLocalPort()).c_str());
        ImGui::NextColumn();
    }
    ImGui::End();
}

void SFMLServerChatApp::ImGuiStats()
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

void SFMLServerChatApp::AcceptClients()
{
    static uint32_t session_id = 0;

    sf::TcpSocket& client = m_ClientSessions[session_id];
    if (m_Listener.accept(client) == sf::Socket::Done)
    {
        client.setBlocking(false);
        session_id++;
    }
}

void SFMLServerChatApp::HandleData()
{
#if TESTING
    for (auto& client : m_ClientSessions)
    {
        sf::TcpSocket& client_socket = client.second;

        sf::Packet packet;
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        if (client_socket.receive(packet) != sf::Socket::Done)
        {
            // error...
            break;
        }
        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        m_TotalTimeReceivingPackets += duration;
        std::string str;
        if (packet >> str) // if correct deserialization of packet
        {
            ++m_TotalNumberOfPacketsReceived;

            std::string message;
            message = "[Client " + std::to_string(client.first);
            message = message + "] " + str;

            AddChatMessage(message);
            std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
            for (auto& other_client : m_ClientSessions)
            {
                sf::Packet send_packet;
                send_packet << message;
                if (other_client.second.send(send_packet) == sf::Socket::Done)
                {
                    ++m_TotalNumberOfPacketsSent;
                }
            }
            std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
            m_TotalTimeSendingPackets += duration;

        }
    }
    m_TextBuffer.clear();
#else
    for (auto& client : m_ClientSessions)
    {
        sf::TcpSocket& client_socket = client.second;

        sf::Packet packet;
        if (client_socket.receive(packet) != sf::Socket::Done)
        {
            // error...
            break;
        }

        std::string str;
        if (packet >> str) // if correct deserialization of packet
        {
            std::string message;
            message = "[Client " + std::to_string(client.first);
            message = message + "] " + str;

            AddChatMessage(message);
            for (auto& other_client : m_ClientSessions)
            {
                sf::Packet send_packet;
                send_packet << message;
                other_client.second.send(send_packet);
            }
        }
    }
#endif
}

void SFMLServerChatApp::AddChatMessage(const std::string& str)
{
    m_TextBuffer.append(str.c_str());
    m_TextBuffer.append("\n");
}
