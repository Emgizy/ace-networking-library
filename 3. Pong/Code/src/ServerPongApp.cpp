#include "ServerPongApp.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"

bool ServerPongApp::Initialize()
{
	bool init = ServerGame::Initialize();

	if (!init)
	{
		return false;
	}

	std::cout << "Server initialized correctly." << std::endl;
	m_Window.create(sf::VideoMode(800, 600), "Technical Computing Project - My Server Pong - Marc Garcia Carda");
	ImGui::SFML::Init(m_Window);

	m_Clock.restart();

	// set ball to half screen
	m_Ball = new Ball({ m_Window.getSize().x / 2.0f, m_Window.getSize().y / 2.0f });
	m_PlayerLeft = new Player({ 100.0f, m_Window.getSize().y / 2.0f });
	m_PlayerRight = new Player({ 700.0f, m_Window.getSize().y / 2.0f });

	m_ClientIdLeft = -1;
	m_ClientIdRight = -1;

	m_ScoreLeft = 0;
	m_ScoreRight = 0;

	return true;
}

void ServerPongApp::Update(float deltaTime)
{
	sf::Time dt = m_Clock.restart();
	deltaTime = dt.asSeconds();

	ServerGame::Update(deltaTime);

	// poll events
	sf::Event event;
	while (m_Window.pollEvent(event))
	{
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
			m_Window.close();
	}

	ImGui::SFML::Update(m_Window, dt);

	// player update
	if (m_PlayerLeft)
	{
		m_PlayerLeft->Update(deltaTime);
		m_PlayerLeft->KeepInsideBounds(m_Window);

		PlayerPositionPacket left_player_packet;
		left_player_packet.m_PlayerType = PlayerType::LEFT;
		left_player_packet.m_Position = m_PlayerLeft->GetPosition();

		m_Server->SendPacketToAllClients(&left_player_packet);
	}

	if (m_PlayerRight)
	{
		m_PlayerRight->Update(deltaTime);
		m_PlayerRight->KeepInsideBounds(m_Window);

		PlayerPositionPacket right_player_packet;
		right_player_packet.m_PlayerType = PlayerType::RIGHT;
		right_player_packet.m_Position = m_PlayerRight->GetPosition();

		m_Server->SendPacketToAllClients(&right_player_packet);
	}

	// ball update only if two players connected
	if (m_Ball && m_ClientIdLeft != -1 && m_ClientIdRight != -1)
	{
		m_Ball->Update(deltaTime);
		m_Ball->KeepInsideBounds(m_Window);

		// check for scoring
		sf::Vector2f position = m_Ball->GetPosition();
		if (position.x < 0.0f)
		{
			++m_ScoreRight;

			SetScorePacket score_packet;
			score_packet.m_PlayerType = PlayerType::RIGHT;
			score_packet.m_Score = m_ScoreRight;
			m_Server->SendPacketToAllClients(&score_packet);

			m_Ball->SetPosition({ m_Window.getSize().x / 2.0f, m_Window.getSize().y / 2.0f });
		}
		if (position.x > m_Window.getSize().x)
		{
			++m_ScoreLeft;

			SetScorePacket score_packet;
			score_packet.m_PlayerType = PlayerType::LEFT;
			score_packet.m_Score = m_ScoreLeft;
			m_Server->SendPacketToAllClients(&score_packet);

			m_Ball->SetPosition({ m_Window.getSize().x / 2.0f, m_Window.getSize().y / 2.0f });
		}

		// if intersects with left, go right
		if (m_Ball->Intersects(m_PlayerLeft))
		{
			sf::Vector2f vel = m_Ball->GetVelocity();
			vel.x = std::abs(vel.x);
			m_Ball->SetVelocity(vel);
		}

		// if intersects with right, go left
		if (m_Ball->Intersects(m_PlayerRight))
		{
			sf::Vector2f vel = m_Ball->GetVelocity();
			vel.x = -std::abs(vel.x);
			m_Ball->SetVelocity(vel);
		}

		BallPositionPacket ball_packet;
		ball_packet.m_Position = m_Ball->GetPosition();
		m_Server->SendPacketToAllClients(&ball_packet);
	}

	// draw
	Draw();
}

void ServerPongApp::Terminate()
{
	// send an terminate connection packet to all clients
	Packet packet;
	packet.m_Type = PacketType::TERMINATE_CONNECTION;
	m_Server->SendPacketToAllClients(&packet);

	delete m_PlayerLeft;
	delete m_PlayerRight;
	delete m_Ball;

	m_Window.close();

	ImGui::SFML::Shutdown();
}

bool ServerPongApp::IsRunning()
{
	return ServerGame::IsRunning() && m_Window.isOpen();
}

int ServerPongApp::HandleSinglePacket(uint32_t client_id, char* data)
{
	Packet packet;
	//find packet type
	packet.Deserialize(data);

	int size = packet.SizeOfPacket();
	switch (packet.m_Type)
	{
	case PacketType::INITIALIZE_CONNECTION:
	{
		// if there is no player on the left
		if (m_ClientIdLeft == -1)
		{
			m_ClientIdLeft = client_id;
			ResetGame();

			SetPlayerControlPacket control_packet;
			control_packet.m_PlayerType = PlayerType::LEFT;
			m_Server->SendPacketToClient(client_id, &control_packet);
			break;
		}

		// if there is no player on the right
		if (m_ClientIdRight == -1)
		{
			m_ClientIdRight = client_id;
			ResetGame();

			SetPlayerControlPacket control_packet;
			control_packet.m_PlayerType = PlayerType::RIGHT;
			m_Server->SendPacketToClient(client_id, &control_packet);
			break;
		}

		// if both players are already playing, add to wait queue
		m_WaitQueue.push_back(client_id);
		// and send updated scores (we dont need to send updated scores
		// in previous cases because we reset and send them)
		SetScorePacket score_packet;
		score_packet.m_PlayerType = PlayerType::RIGHT;
		score_packet.m_Score = m_ScoreRight;
		m_Server->SendPacketToAllClients(&score_packet);
		score_packet.m_PlayerType = PlayerType::LEFT;
		score_packet.m_Score = m_ScoreLeft;
		m_Server->SendPacketToAllClients(&score_packet);
		break;
	}
	break;
	case PacketType::TERMINATE_CONNECTION:
	{
		m_Server->RemoveClient(client_id);
		// if it was a client waiting in queue, erase it
		const auto& it = std::find(m_WaitQueue.begin(), m_WaitQueue.end(), client_id);
		if (it != m_WaitQueue.end())
		{
			m_WaitQueue.erase(it, it + 1);
			break;
		}

		// if it was the left player
		if (m_ClientIdLeft == client_id)
		{
			// find if there was someone waiting
			if (!m_WaitQueue.empty())
			{
				m_ClientIdLeft = m_WaitQueue.front();
				m_WaitQueue.pop_front();


				SetPlayerControlPacket control_packet;
				control_packet.m_PlayerType = PlayerType::LEFT;
				m_Server->SendPacketToClient(m_ClientIdLeft, &control_packet);
			}
			else
			{
				m_ClientIdLeft = -1;
			}
		}

		// if it was the right player
		if (m_ClientIdRight == client_id)
		{
			// find if there was someone waiting
			if (!m_WaitQueue.empty())
			{
				m_ClientIdRight = m_WaitQueue.front();
				m_WaitQueue.pop_front();

				SetPlayerControlPacket control_packet;
				control_packet.m_PlayerType = PlayerType::RIGHT;
				m_Server->SendPacketToClient(m_ClientIdRight, &control_packet);
			}
			else
			{
				m_ClientIdRight = -1;
			}
		}
		ResetGame();

	}
	break;
	case PacketType::PLAYER_VELOCITY:
	{
		PlayerVelocityPacket player_packet;
		player_packet.Deserialize(data);

		switch (player_packet.m_PlayerType)
		{
		case PlayerType::LEFT:
		{
			if (m_PlayerLeft)
			{
				m_PlayerLeft->SetVelocity(player_packet.m_Velocity);
			}
		}
		break;
		case PlayerType::RIGHT:
		{
			if (m_PlayerRight)
			{
				m_PlayerRight->SetVelocity(player_packet.m_Velocity);
			}
		}
		break;
		default:
			break;
		}

		size = player_packet.SizeOfPacket();
	}
	break;
	default:
		break;
	}

	return size;
}

void ServerPongApp::Draw()
{
	DrawImGui();

	m_Window.clear(sf::Color::Black);


	// Draw all elements
	if (m_Ball)
	{
		m_Ball->Draw(m_Window);
	}

	if (m_PlayerLeft)
	{
		ImGui::SetNextWindowPos({100.0f, 0.0f }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({100.0f, 50.0f}, ImGuiCond_Once);
		ImGui::Begin("P1",  nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::Text("Player 1");
		ImGui::Text(std::to_string(m_ScoreLeft).c_str());

		ImGui::End();

		m_PlayerLeft->Draw(m_Window);
	}

	if (m_PlayerRight)
	{
		ImGui::SetNextWindowPos({ 650.0f, 0.0f }, ImGuiCond_Once);
		ImGui::SetNextWindowSize({ 100.0f, 50.0f }, ImGuiCond_Once);
		ImGui::Begin("P2", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::Text("Player 2");
		ImGui::Text(std::to_string(m_ScoreRight).c_str());

		ImGui::End();

		m_PlayerRight->Draw(m_Window);
	}

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

void ServerPongApp::DrawImGui()
{
	ImGui::Begin("Wait Queue");
	for (const auto& client : m_WaitQueue)
	{
		ImGui::Text(std::to_string(client).c_str());
	}
	ImGui::End();


	ImGui::Begin("Players");
	std::string left_text = "Left: " + std::to_string(m_ClientIdLeft);
	std::string right_text = "Right: " + std::to_string(m_ClientIdRight);
	ImGui::Text(left_text.c_str());
	ImGui::Text(right_text.c_str());
	ImGui::End();
}

void ServerPongApp::ResetGame()
{
	m_ScoreLeft = 0;
	{
		SetScorePacket score_packet;
		score_packet.m_PlayerType = PlayerType::LEFT;
		score_packet.m_Score = m_ScoreLeft;
		m_Server->SendPacketToAllClients(&score_packet);
	}
	m_ScoreRight = 0;
	{
		SetScorePacket score_packet;
		score_packet.m_PlayerType = PlayerType::RIGHT;
		score_packet.m_Score = m_ScoreRight;
		m_Server->SendPacketToAllClients(&score_packet);
	}


	m_Ball->SetPosition({ m_Window.getSize().x / 2.0f, m_Window.getSize().y / 2.0f });
}
