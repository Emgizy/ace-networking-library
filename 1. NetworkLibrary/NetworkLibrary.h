#pragma once

// port
#define DEFAULT_PORT "8888"

// host
#define DEFAULT_HOST "127.0.0.1" // "127.0.0.1" for localhost

// max packet size
#define MAX_PACKET_SIZE 1000000

// windows includes
#include <winsock2.h>

#include <ws2tcpip.h>

#include <Windows.h>

// other stl utilities
#include <iostream>

#include <string>

#include <map>

// link with libraries
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#pragma comment(lib, "Ws2_32.lib")

// base enum for packet types
enum class PacketType {
    INITIALIZE_CONNECTION = 0,
    TERMINATE_CONNECTION,
    MESSAGE,
    DEFAULT,
};

// base packet class
class Packet {
public:
    Packet() : m_Type(PacketType::DEFAULT) {}
    Packet(PacketType type) : m_Type(type) {}
    virtual~Packet() =
        default;

    PacketType m_Type;

    virtual void Serialize(char* data) {
        memcpy(data, &m_Type, sizeof(PacketType));
    }

    virtual void Deserialize(char* data) {
        memcpy(&m_Type, data, sizeof(PacketType));
    }

    virtual int SizeOfPacket() const {
        return sizeof(PacketType);
    }
};

// example derived packet class
class MessagePacket : public Packet {
public: MessagePacket() : Packet(PacketType::MESSAGE) {}

      std::string m_Message;

      virtual void Serialize(char* data) override {
          memcpy(data, &m_Type, sizeof(PacketType)); //first serialize the type

          size_t length = m_Message.length();
          memcpy(data + sizeof(PacketType), &length, sizeof(size_t)); //then serialize the length of the string

          const char* string = m_Message.c_str();
          memcpy(data + sizeof(PacketType) + sizeof(size_t), string, length); //finally serialize the string itself
      }

      virtual void Deserialize(char* data) override {
          memcpy(&m_Type, data, sizeof(PacketType)); //first deserialize the type

          size_t length;
          memcpy(&length, data + sizeof(PacketType), sizeof(size_t)); //then deserialize the length of the string

          char* string = new char[length + 1]; //+1 to accomodate for null termination
          memset(string, 0, length + 1);
          memcpy(string, data + sizeof(PacketType) + sizeof(size_t), length); //finally serialize the string itself

          m_Message = string;
          delete[] string;
      }

      virtual int SizeOfPacket() const override {
          return sizeof(PacketType) + sizeof(size_t) + m_Message.length();
      }
};

// base client class
class Client {
public:
    Client() {
        // create empty WSADATA object
        WSADATA wsaData;

        // set socket to invalid
        m_Socket = INVALID_SOCKET;

        // initialize Winsock
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (res != 0) {
            WSACleanup();
            std::string msg = "Winsock initialization failed:" + std::to_string(res);
            throw std::exception(msg.c_str());
        }

        // address info for socket
        addrinfo* addr_result = nullptr;
        addrinfo* addr_ptr = nullptr;
        addrinfo addr_hints;

        // set address info
        memset(&addr_hints, 0, sizeof(addr_hints));
        addr_hints.ai_family = AF_UNSPEC;
        addr_hints.ai_socktype = SOCK_STREAM;
        addr_hints.ai_protocol = IPPROTO_TCP;

        // get address info
        res = getaddrinfo(DEFAULT_HOST, DEFAULT_PORT, &addr_hints, &addr_result);

        if (res != 0) {
            WSACleanup();
            std::string msg = "getaddrinfo failed:" + std::to_string(res);
            throw std::exception(msg.c_str());
        }

        // connecto to address 
        for (addr_ptr = addr_result; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next) {

            // create socket
            m_Socket = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol);

            // if invalid, clean up
            if (m_Socket == INVALID_SOCKET) {
                WSACleanup();
                std::string msg = "Socket creation failed:" + std::to_string(res);
                throw std::exception(msg.c_str());
            }

            // try and connect to a server
            res = connect(m_Socket, addr_ptr->ai_addr, addr_ptr->ai_addrlen);

            if (res == SOCKET_ERROR) {
                closesocket(m_Socket);
                m_Socket = INVALID_SOCKET;
                std::cout << "Socket error, did not connect." << std::endl;
            }
        }

        // free the address info
        freeaddrinfo(addr_result);

        // fail
        if (m_Socket == INVALID_SOCKET) {
            WSACleanup();
            throw std::exception("Connection failed");
        }

        // non-blocking mode for socket
        u_long nb_mode = 1;
        res = ioctlsocket(m_Socket, FIONBIO, &nb_mode);
        if (res == SOCKET_ERROR) {
            closesocket(m_Socket);
            WSACleanup();
            std::string msg = "ioctlsocket failed: " + std::to_string(WSAGetLastError());
            throw std::exception(msg.c_str());
        }
    }

    ~Client() {
        // shutdown the socket
        shutdown(m_Socket, SD_SEND);

        // close our socket
        closesocket(m_Socket);

        // cleanup WinSock
        WSACleanup();
    }

    // sends a specific packet to its server, returning
    // the total number of bytes sent or an error
    int SendPacketToServer(Packet* packet) {
        // find packet size
        const size_t packet_size = packet->SizeOfPacket();

        // create buffer for data and set to 0
        char* packet_data = new char[packet_size];
        memset(packet_data, 0, packet_size);

        // serialize the data into buffer
        packet->Serialize(packet_data);

        // send it without specific flags
        int sending = send(m_Socket, packet_data, packet_size, 0);

        // cleanup pointers
        delete[] packet_data;
        return sending;
    }

    // receives data from the server, is non-blocking
    int ReceiveDataFromServer(char* receiveBuffer) {
        int result = 0;
        int rec = 0;
        do {
            rec = recv(m_Socket, receiveBuffer + result, MAX_PACKET_SIZE, 0);
            result += rec;
        } while (rec > 0);
        return result - rec;
    }

private:
    // client socket
    SOCKET m_Socket;
};

// base server class
class Server {
public:
    Server() {
        // create empty WSADATA object
        WSADATA wsaData;

        // listening server socket
        m_ListenSocket = INVALID_SOCKET;

        // initialize Winsock
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (res != 0) {
            WSACleanup();
            std::string msg = "Winsock initialization failed:" + std::to_string(res);
            throw std::exception(msg.c_str());
        }

        // address info for socket
        struct addrinfo* addr_result = NULL;
        struct addrinfo addr_hints;

        // set address info
        memset(&addr_hints, 0, sizeof(addr_hints));
        addr_hints.ai_family = AF_INET;
        addr_hints.ai_socktype = SOCK_STREAM;
        addr_hints.ai_protocol = IPPROTO_TCP;
        addr_hints.ai_flags = AI_PASSIVE;

        // get address info
        res = getaddrinfo(NULL, DEFAULT_PORT, &addr_hints, &addr_result);

        if (res != 0) {
            WSACleanup();
            std::string msg = "getaddrinfo failed:" + std::to_string(res);
            throw std::exception(msg.c_str());
        }

        // create listening socket
        m_ListenSocket = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);

        // if invalid, clean up
        if (m_ListenSocket == INVALID_SOCKET) {
            freeaddrinfo(addr_result);
            WSACleanup();
            std::string msg = "Socket creation failed:" + std::to_string(WSAGetLastError());
            throw std::exception(msg.c_str());
        }

        // non-blocking mode for socket
        u_long nb_mode = 1;
        res = ioctlsocket(m_ListenSocket, FIONBIO, &nb_mode);
        if (res == SOCKET_ERROR) {
            closesocket(m_ListenSocket);
            WSACleanup();
            std::string msg = "ioctlsocket failed: " + std::to_string(WSAGetLastError());
            throw std::exception(msg.c_str());
        }

        // bind TCP listening socket
        res = bind(m_ListenSocket, addr_result->ai_addr, addr_result->ai_addrlen);

        if (res == SOCKET_ERROR) {
            freeaddrinfo(addr_result);
            closesocket(m_ListenSocket);
            WSACleanup();
            std::string msg = "Bind failed: " + std::to_string(WSAGetLastError());
            throw std::exception(msg.c_str());
        }
        freeaddrinfo(addr_result);

        // listen for clients
        res = listen(m_ListenSocket, SOMAXCONN);

        if (res == SOCKET_ERROR) {
            closesocket(m_ListenSocket);
            WSACleanup();
            std::string msg = "Listen failed: " + std::to_string(WSAGetLastError());
            throw std::exception(msg.c_str());
        }
    }

    ~Server() {
        // remove all clients
        auto copy_client_session = m_ClientSessions;
        for (const auto& client : copy_client_session) {
            RemoveClient(client.first);
        }

        // Shutdown the socket.
        shutdown(m_ListenSocket, SD_SEND);

        // Close our socket entirely.
        closesocket(m_ListenSocket);

        // Cleanup Winsock - release any resources used by Winsock.
        WSACleanup();
    }

    // accept new clients to map
    bool AcceptNewClient(uint32_t id) {
        // if client waiting, accept the connection and save the socket
        SOCKET client_socket = accept(m_ListenSocket, NULL, NULL);

        if (client_socket == INVALID_SOCKET) {
            return false;
        }

        // add client
        m_ClientSessions.insert({
          id,
          client_socket
            });

        return true;
    }

    // remove the client from our map and close it's socket
    // sending a TERMINATE_CONNECTION Packet to client
    void RemoveClient(uint32_t id) {
        auto it = m_ClientSessions.find(id);
        if (it == m_ClientSessions.end())
            return;

        Packet packet(PacketType::TERMINATE_CONNECTION);
        SendPacketToClient(id, &packet);

        SOCKET& client_socket = m_ClientSessions[id];
        closesocket(client_socket);
        m_ClientSessions.erase(it);
    }

    // receive data from a specific client, is non-blocking
    int ReceiveDataFromClient(uint32_t clientID, char* receiveBuffer) {
        if (m_ClientSessions.find(clientID) != m_ClientSessions.end()) {
            SOCKET currentSocket = m_ClientSessions[clientID];

            int result = 0;
            int rec = 0;
            do {
                rec = recv(currentSocket, receiveBuffer + result, MAX_PACKET_SIZE, 0);
                result += rec;
            } while (rec > 0);
            return result - rec;
        }
        return 0;
    }

    // sends data to a specific client
    int SendPacketToClient(uint32_t id, Packet* packet) {
        auto it = m_ClientSessions.find(id);
        if (it == m_ClientSessions.end())
            return -1;

        SOCKET& client_socket = m_ClientSessions[id];

        const size_t size = packet->SizeOfPacket();
        char* data = new char[size];
        memset(data, 0, size);

        packet->Serialize(data);
        int sending = send(client_socket, data, size, 0);
        delete[] data;
        return sending;
    }

    // sends data to all clients
    int SendPacketToAllClients(Packet* packet) {
        int result = 0;
        for (const auto& client : m_ClientSessions) {
            result += SendPacketToClient(client.first, packet);
        }
        return result;
    }

    // listening socket
    SOCKET m_ListenSocket;

    // map from id to socket
    std::map < uint32_t, SOCKET > m_ClientSessions;
};

// base clientgame loop
class ClientGame {
public:
    // initializes de client class and starts running the application
    virtual bool Initialize() {
        m_Client = std::make_unique < Client >();
        m_IsRunning = true;

        return IsClientValid();
    }

    // base update, receives data from the server and calls
    // HandleSinglePacket for each packet
    virtual void Update(float) {
        ClientGame::ReceiveDataFromServer();
    }

    // stops the application and deletes pertinent information
    virtual void Terminate() {
        m_IsRunning = false;
    }

    // returns if the application is still running
    virtual bool IsRunning() {
        return m_IsRunning;
    }

protected:
    ClientGame() :
        m_IsRunning(false) {}

    virtual~ClientGame() {}

    virtual bool IsClientValid() {
        return m_Client.get();
    }

    // handles a single packet of data and returns
    // the size of that packet
    virtual int HandleSinglePacket(char* data) {
        Packet packet;

        packet.Deserialize(data);

        int size = packet.SizeOfPacket();
        switch (packet.m_Type) {
        case PacketType::INITIALIZE_CONNECTION:
            std::cout << "Server has sent INITIALIZE_CONNECTION packet to the client." << std::endl;
            break;
        case PacketType::TERMINATE_CONNECTION:
            std::cout << "Server has sent TERMINATE_CONNECTION packet to the client." << std::endl;
            break;
        default:
            break;
        }

        return size;
    }

    // receives all data from the server, is not blocking
    virtual void ReceiveDataFromServer() {
        // get data sent by that client
        int data_length = m_Client->ReceiveDataFromServer(m_NetworkData);

        if (data_length <= 0) {
            // there is no data received
            return;
        }

        int i = 0;
        while (i < data_length) {
            int size = HandleSinglePacket(&m_NetworkData[i]);
            i += size;
        }
    }

protected:
    std::unique_ptr < Client > m_Client;

    // is the application running
    bool m_IsRunning;

    // data buffer
    char m_NetworkData[MAX_PACKET_SIZE] = {
      0
    };
};

// base servergame loop
class ServerGame {
public:
    virtual bool Initialize() {
        m_Server = std::make_unique < Server >();
        m_IsRunning = true;

        return IsServerValid();
    }

    virtual void Update(float) {
        ServerGame::AcceptingNewClients();
        ServerGame::ReceiveDataFromClients();
    }

    virtual void Terminate() {

    }
    virtual bool IsRunning() {
        return m_IsRunning;
    }

protected:
    ServerGame() :
        m_LastClientId(0),
        m_IsRunning(false) {

    }

    virtual~ServerGame() {}

    virtual bool IsServerValid() {
        return m_Server.get();
    }

    virtual void AcceptingNewClients() {
        if (!IsServerValid()) //if server not valid
        {
            std::cout << "Server not valid!" << std::endl;
            return;
        }

        if (m_Server->AcceptNewClient(m_LastClientId)) {
            m_LastClientId++;
        }
    }

    // return size of the actual package read
    virtual int HandleSinglePacket(uint32_t client_id, char* data) {
        Packet packet;

        packet.Deserialize(data);

        int size = packet.SizeOfPacket();
        switch (packet.m_Type) {
        case PacketType::INITIALIZE_CONNECTION:
            std::cout << "Client " << client_id << "has sent INITIALIZE_CONNECTION packet to the server." << std::endl;
            break;
        case PacketType::TERMINATE_CONNECTION:
            std::cout << "Client " << client_id << " has sent TERMINATE_CONNECTION packet to the server." << std::endl;
            break;
        default:
            break;
        }

        return size;
    }

    virtual void ReceiveDataFromClients() {
        // copy so that we can erase clients on the fly
        const auto client_sessions = m_Server->m_ClientSessions;
        for (const auto& client : client_sessions) {
            // get data sent by that client
            int data_length = m_Server->ReceiveDataFromClient(client.first, m_NetworkData);

            if (data_length <= 0) {
                // there is no data received
                continue;
            }

            int i = 0;
            while (i < data_length) {
                int size = HandleSinglePacket(client.first, &(m_NetworkData[i]));
                i += size;
            }
        }

    }

protected:
    uint32_t m_LastClientId;
    std::unique_ptr < Server > m_Server;

    // is the application running
    bool m_IsRunning;

    // data buffer
    char m_NetworkData[MAX_PACKET_SIZE] = {
      0
    };
};
