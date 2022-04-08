#include "ClientPongApp.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"

bool ClientPongApp::Initialize()
{
	bool init = ClientGame::Initialize();
	if (!init)
	{
		return false;
	}

	std::cout << "Client initialized correctly." << std::endl;

	m_Window.create(sf::VideoMode(800, 600), "Technical Computing Project - My Client Pong - Marc Garcia Carda");
	ImGui::SFML::Init(m_Window);

	m_Clock.restart();

	// send an initialize connection packet to server
	Packet packet(PacketType::INITIALIZE_CONNECTION);

	int sending = m_Client->SendPacketToServer(&packet);

	m_Ball = new Ball({ m_Window.getSize().x / 2.0f, m_Window.getSize().y / 2.0f });
	m_PlayerLeft = new Player({ 100.0f, m_Window.getSize().y / 2.0f });
	m_PlayerRight = new Player({ 700.0f, m_Window.getSize().y / 2.0f });

	m_PlayerType = PlayerType::NONE;

	m_ScoreLeft = 0;
	m_ScoreRight = 0;

	return sending != SOCKET_ERROR;
}

void ClientPongApp::Update(float deltaTime)
{
	sf::Time dt = m_Clock.restart();
	deltaTime = dt.asSeconds();

	ClientGame::Update(deltaTime);

	// poll events
	sf::Event event;
	while (m_Window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
			m_Window.close();
	}
	ImGui::SFML::Update(m_Window, dt);

	PlayerInput();

	Draw();
}

void ClientPongApp::Terminate()
{
	// send an terminate connection packet to server
	Packet packet;
	packet.m_Type = PacketType::TERMINATE_CONNECTION;
	m_Client->SendPacketToServer(&packet);

	m_Window.close();
	ImGui::SFML::Shutdown();

	delete m_Ball;
	delete m_PlayerLeft;
	delete m_PlayerRight;
}

bool ClientPongApp::IsRunning()
{
	return ClientGame::IsRunning() && m_Window.isOpen();
}

int ClientPongApp::HandleSinglePacket(char* data)
{
	Packet packet;

	packet.Deserialize(data);

	int size = packet.SizeOfPacket();
	switch (packet.m_Type)
	{
	case PacketType::INITIALIZE_CONNECTION:
	{

	}
	break;
	case PacketType::TERMINATE_CONNECTION:
	{
		
	}
	break;
	case PacketType::BALL_POSITION:
	{
		BallPositionPacket ball_packet;
		ball_packet.Deserialize(data);
		if (m_Ball)
		{
			m_Ball->SetPosition(ball_packet.m_Position);
		}
		size = ball_packet.SizeOfPacket();
	}
	break;
	case PacketType::PLAYER_POSITION:
	{
		PlayerPositionPacket player_packet;
		player_packet.Deserialize(data);
		size = player_packet.SizeOfPacket();

		switch (player_packet.m_PlayerType)
		{
		case PlayerType::LEFT:
		{
			if (m_PlayerLeft)
			{
				m_PlayerLeft->SetPosition(player_packet.m_Position);
			}
		}
		break;
		case PlayerType::RIGHT:
		{
			if (m_PlayerRight)
			{
				m_PlayerRight->SetPosition(player_packet.m_Position);
			}
		}
		break;
		default:
			break;
		}

	}
	break;
	case PacketType::PLAYER_CONTROL:
	{
		SetPlayerControlPacket player_control_packet;
		player_control_packet.Deserialize(data);
		size = player_control_packet.SizeOfPacket();

		m_PlayerType = player_control_packet.m_PlayerType;
		if (m_PlayerType == PlayerType::LEFT)
		{
			m_PlayerLeft->SetSelected(true);
			m_PlayerRight->SetSelected(false);
		}
		if (m_PlayerType == PlayerType::RIGHT)
		{
			m_PlayerLeft->SetSelected(false);
			m_PlayerRight->SetSelected(true);
		}

	}
	break;
	case PacketType::SCORE:
	{
		SetScorePacket score_packet;
		score_packet.Deserialize(data);
		size = score_packet.SizeOfPacket();

		if (score_packet.m_PlayerType == PlayerType::LEFT)
		{
			m_ScoreLeft = score_packet.m_Score;
		}
		if (score_packet.m_PlayerType == PlayerType::RIGHT)
		{
			m_ScoreRight = score_packet.m_Score;
		}

	}
	break;
	default:
		break;
	}

	return size;
}

void ClientPongApp::PlayerInput()
{
	//check for movement
	Player* player = GetCurrentPlayer();
	if (!player) return;

	float speed = 0.0f;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		speed -= 150.0f;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		speed += 150.0f;
	}

	PlayerVelocityPacket player_packet;
	player_packet.m_PlayerType = m_PlayerType;
	player_packet.m_Velocity = speed;
	m_Client->SendPacketToServer(&player_packet);
}

void ClientPongApp::Draw()
{
	m_Window.clear(sf::Color::Black);


	// Draw all elements
	if (m_Ball)
	{
		m_Ball->Draw(m_Window);
	}

	if (m_PlayerLeft)
	{
		ImGui::SetNextWindowPos({ 100.0f, 0.0f }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({ 100.0f, 50.0f }, ImGuiCond_Once);
		ImGui::Begin("P1", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		if (m_PlayerType == PlayerType::LEFT)
		{
			ImGui::Text("YOU");

		}
		else
		{
			ImGui::Text("Player 1");
		}
		ImGui::Text(std::to_string(m_ScoreLeft).c_str());

		ImGui::End();

		m_PlayerLeft->Draw(m_Window);
	}

	if (m_PlayerRight)
	{
		ImGui::SetNextWindowPos({ 650.0f, 0.0f }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({ 100.0f, 50.0f }, ImGuiCond_Once);
		ImGui::Begin("P2", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		if (m_PlayerType == PlayerType::RIGHT)
		{
			ImGui::Text("YOU");

		}
		else
		{
			ImGui::Text("Player 2");
		}
		ImGui::Text(std::to_string(m_ScoreRight).c_str());

		ImGui::End();

		m_PlayerRight->Draw(m_Window);
	}


	ImGui::SetNextWindowPos({ m_Window.getSize().x / 2.0f - 50.0f, 0.0f }, ImGuiCond_Once);
	ImGui::SetNextWindowSize({ 100.0f, 50.0f }, ImGuiCond_Once);
	ImGui::Begin("Spectator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	if (m_PlayerType == PlayerType::NONE)
	{
		ImGui::Text("Spectating");
	}
	else
	{
		ImGui::Text("Playing");
	}
	ImGui::End();


	//DrawLines
	{
		sf::Vertex half_mark_line[] =
		{
			sf::Vertex(sf::Vector2f((float)m_Window.getSize().x / 2.0f, 100.0f)),
			sf::Vertex(sf::Vector2f(m_Window.getSize().x / 2.0f, (float)m_Window.getSize().y))
		};

		sf::Vertex UI_line[] =
		{
			sf::Vertex(sf::Vector2f(0.0f, 100.0f)),
			sf::Vertex(sf::Vector2f((float)m_Window.getSize().x, 100.0f))
		};

		m_Window.draw(half_mark_line, 2, sf::Lines);
		m_Window.draw(UI_line, 2, sf::Lines);
	}

	ImGui::SFML::Render(m_Window);

	m_Window.display();
}

Player* ClientPongApp::GetCurrentPlayer()
{
	switch (m_PlayerType)
	{
	case PlayerType::LEFT:
		return m_PlayerLeft;
		break;
	case PlayerType::RIGHT:
		return m_PlayerRight;
		break;
	case PlayerType::NONE:
		return nullptr;
		break;
	default:
		break;
	}
	return nullptr;
}
