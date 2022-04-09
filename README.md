# ACE 
## *C++ Networking Library for Windows*

![ACE-logo](https://user-images.githubusercontent.com/48070316/162492398-7d98083e-c759-4ca9-8bbe-b3886d7fb156.jpeg)

Welcome to ACE, an Accessible, Clean and Easy-to-use C++ networking library for Windows applications.

This project started in 2021 as part of my last year's Technical Computing Project at Sheffield Hallam University with the goal of furthering my understanding of networking and on-line programming.

## *Getting started*
Due to ACE being designed as a single-header-library, to begin using it you will only have to copy the "NetworkLibrary.h" file found inside the NetworkLibrary folder to your own include directory, and add it into your project files using
```cpp
#include "NetworkLibrary.h"
```
That's it.

No strange build commands.

No unnecessary compiled libraries.

Nothing.

See?

Accessible, clean and easy-to-use.

## *How To Use*
There are three main classes to understand in this library: the Packet, the Server and the Client.

### Packet
The Packet class is just our way to abstract the data we will be sending. We will have an PacketType enum that will differentiate between types of Packets. Obviously.

The basic methods of this packet class include Serialize (convert all its data into a stream of bytes), Deserialize (construct itself from a stream of bytes) and SizeOfPacket (know how big the packet will actually be).

```cpp
  char data[MAX_PACKET_SIZE] = {0};
  
  Packet p1(PacketType::DEFAULT);
  p1.Serialize(data);                 // convert its information into the stream of bytes "data"
  
  Packet p2;
  p2.Deserialize(data);               // construct itself from the stream "data"
```
This base class will only contain a type, so when we send it we will basically be sending a "flag" indicating a state or change of state. This can be to alert of a new connection, terminate an existing connection, pause the application or any other single notification we would want. More complex packets can be sent, such as packets containing a message, a 2D position, a vector of highscores, etc... but these will be covered later.

### Server
To create a server, the code might seem difficult, but trust me, its simpler than it seems and we'll go through it line by line.

```cpp
  Server server; // Create a server
```

What? You think that's difficult? We might have to talk a bit about your programming experience then...

To create and open a server it is as simple as instantiating a server class. This server will be opened at the DEFAULT_PORT specified on the top of the "NetworkingLibrary.h" file as a non-blocking TCP socket (all of you networking geniuses know what I'm talking about).

This server has the option to accept clients, remove them, send data to one client in particular or to all of them and receive data back from them.

To accept clients just add the AcceptNewClient method into you loop and add the id you want the new client to take.

```cpp
  uint32_t clientId = 0;                  // the first client can have an ID of 0
  while(isRunning)
  {
    if(server.AcceptNewClient(clientId))  // this method returns true if the client is found
    {
      ++clientId;                         // lets keep ids unique
    }
  
    // do some more stuff
  }
```

And to remove them, just use the RemoveClient method with the corresponding client id.

```cpp
  server.RemoveClient(4); // we don't like client 4 for some reason... we prefer client 3
```

Then, to send data we will use a Packet class; just create an instance of the Packet class (or a class that derives from it), populate with the pertinent info and then send it using the SendPacketToClient or SendPacketToAllClients methods.

```cpp
  Packet default_packet(PacketType::DEFAULT);
  server.SendPacketToClient(2, &default_packet);          // lets just remind client 2 that we're still here
  
  MessagePacket msg_packet;                               // more info on this packet later on, for now just know it will send an std::string
  msg_packet.m_Message = "We love you the most.";         // the message will say that we love client 3 the most
  server.SendPacketToClient(3, &msg_packet);              // and so we have to remind them of it
  
  Packet term_packet(PacketType::TERMINATE_CONNECTION);
  server.SendPacketToAllClients(&term_packet);            // quick, lets terminate all their connections before they notice our preferences
```

Finally, to receive data from a client, we just have to provide the ReceiveDataFromClient with their ID as well as a big enough buffer to contain all info. In this case, we can use the predefined size of MAX_PACKET_SIZE to create our buffer.

```cpp
  char data[MAX_PACKET_SIZE] = {0};                       // good programers initialize their variables
                                                          
  int num_bytes = server.ReceiveDataFromClient(3, data);  // the method returns the number of bytes received
  if(num_bytes > 0)     // lets see if we have heard back from client 3 :)
  {
    // we've got info waiting for us inside data!!
  }
  else
  {
    // sad... he hasn't said anything back yet :(
  }
```

### Client

Clients are as simple as Servers, in their creation, sending packets and receiving data. Just take a look at this sample code and tell me I'm lying.

```cpp
  // creating a client
  Client client;
  
  // sending data to the server
  MessagePacket msg_packet;
  msg_packet.m_Message = "I love you too, server."; 
  client.SendPacketToServer(&msg_packet); 
  
  // receiving data
  char data[MAX_PACKET_SIZE] = {0};
                                                          
  int num_bytes = client.ReceiveDataFromServer(data)
  if(num_bytes > 0)
  {
    // we got info
  }
  else
  {
    // no info this time
  }
```
These clients will try to connect to the DEFAULT_HOST defined at the beggining of the "NetworkLibrary.h" file. It's default is "127.0.0.1" for localhost, but you may find your own IP multiple ways (I like to use this website for a quick way to find it using the windows cmd tool: https://www.avast.com/c-how-to-find-ip-address)

## *Client and Server Loops*

As the library stands right now, the server is created whenever a "Server" class is created, and destroyed when it's destroyed. The same goes for clients, as they try to connect to the server once when they are created, and will throw an exception if this fails. This will be improved in the future, but, for now, we will use the tools we have available.

To make your life as a programmer easier there is a custom "ClientGame" and "ServerGame" classes that serve as interfaces for basic functions the application may have. Each has a copy of either a Client or a Server, and will take care of most of the networking stuff for you. The only things left for you are to override the "HandleSinglePacket" function which will receive a buffer of bytes (in this case, a char*) and decide what to do with it.

This might seem very unintuitive, but packets are built so that the first bytes represent the PacketType enum, so a simple switch statement must suffice, such as the following

```cpp
  virtual int HandleSinglePacket(char * data) {
    Packet packet;

    packet.Deserialize(data);

    int size = packet.SizeOfPacket();
    switch (packet.m_Type) {
    case PacketType::INITIALIZE_CONNECTION:
      // do initialize connection stuff
      break;
    case PacketType::TERMINATE_CONNECTION:
      // do terminate connection stuff
      break;
    case PacketType::MESSAGE:
      // do message stuff
      break;
    default:
      break;
    }

    return size;
  }
```
If your packets have more data than the single "flag" of their type, then you can get this data by deserializing them. This will be further explained in the [Custom Packets](#-*Custom-Packets*) section.

Sending data is very similar to the previous ways we have stated, just changing your "client" or "server" variables to the corresponding "m_Client" or "m_Server" class member variables.

## *Custom Packets*

To create custom packets, you will need to create a class that derives from the base Packet class, and overrides its virtual functions (Serialize, Deserialize and SizeOfPacket), as well as adding an enum to the PacketType enum.
An example "Message Packet" class is included, in which we send an std::string.
```cpp
class MessagePacket: public Packet {
  public: MessagePacket(): Packet(PacketType::MESSAGE) {}

  std::string m_Message;

  virtual void Serialize(char * data) override {
    memcpy(data, & m_Type, sizeof(PacketType));

    size_t length = m_Message.length();
    memcpy(data + sizeof(PacketType), & length, sizeof(size_t));

    const char * string = m_Message.c_str();
    memcpy(data + sizeof(PacketType) + sizeof(size_t), string, length);
  }

  virtual void Deserialize(char * data) override {
    memcpy( & m_Type, data, sizeof(PacketType));

    size_t length;
    memcpy( & length, data + sizeof(PacketType), sizeof(size_t));

    char * string = new char[length + 1];
    memset(string, 0, length + 1);
    memcpy(string, data + sizeof(PacketType) + sizeof(size_t), length);

    m_Message = string;
    delete[] string;
  }

  virtual int SizeOfPacket() const override {
    return sizeof(PacketType) + sizeof(size_t) + m_Message.length();
  }
};
```

Then, to send a custom packet it's as easy as sending a basic Packet, just with the derived class.

```cpp
  MessagePacket msg;
  msg.m_Message = "Hello World!";
  client.SendPacketToServer(&msg);
```

And to handle receving it, just add another case to your HandleSinglePacket switch statement.
```cpp
  case PacketType::MESSAGE:
  {
    MessagePacket msg;
    msg.Deserialize(data);
    size = msg.SizeofPacket();
  
    std::cout << "A message packet containing: " << msg.m_Message << " has been received.";
  }
```


## *Demos*
Included with this library are two demos which replicate a messaging chat and a Pong clone so that you can test them out for yourself. To build any of the Demo's you can either use the Executable folder to run it or build it using the "compile_vs20XX" batch file, which will run premake on the lua script and generate a Visual Studio solution. Remember that the defaults for everything are "localhost" so if you don't change them your computer must first execute a "Server" and then as many "Clients" as you'd like.

As this project goes on further, more demos will be created and uploaded for use and inspiration.

### Chat
A simple chat executable, where clients can connect to a server and send messages to one another.

This demo was created to iron out the simplest bugs in the library and was used to test its speed against other libraries. 
![Chat](https://user-images.githubusercontent.com/48070316/162490711-6957dc8c-b428-46e5-9204-089c173cc10c.png)

### Pong
An online recreation of the classic 1972 Atari arcade game; Pong.

This demo was centred around testing if the library’s design was actually usable for a game deliverable and tried to expose any design flaws that the library could have, as well as where it can be improved in the future.
![Pong](https://user-images.githubusercontent.com/48070316/162490534-f90b9732-745c-40f3-b5b0-4ed24386a077.png)

## *Future Improvements*
- API rework
  - IP address and port moved from #defines to variables
  - Initialize server
  - Accept new client
  - Client reconnect to server
  - Send packet to all clients except for X
  - Add ACE namespace
- More demos
  - Server lobby with multiple servers
- More packet types
  - Raw data packet (char* and size)
- In depth documentation
  - Readme
  - Documentation
  - Wiki
  - Port forwarding

## *Acknowledgements*

Thanks to Luke Melville and Paul Parry for their unbelievable help in understanding the topic at hand.


## *References*
1.	Lai, Ali, A., Hossain, M. S., & Lin, Y.-D. (2019). Performance modeling and analysis of TCP and UDP flows over software defined networks. Journal of Network and Computer Applications, 130, 76–88.
2.	Spragins, J. (1996). Internetworking with TCP/IP, Volume III: Client-server programming and applications, second edition. IEEE Network, Vol. 10 (4), p.4
3.	Van Winkle, L. (2019). Hands-on network programming with C : learn socket programming in C and write secure and optimized network code.
4.	Schmidt, D.; Huston, S. (2002). C++ Network Programming.
5.	Bidgoli, H. (2008). Handbook of computer networks. volume 2, LANs, MANs, WANs, the Internet, and Global, Cellular, and Wireless Networks. Pg 3.
6.	Dostálek, L.; Kabelová, A. (2006) Understanding TCP/IP a clear and comprehensive guide to TCP/IP protocols.
7.	Donahoo, Michael J.; Calvert, Kenneth L.; Donahoo, Michael J. (2009). TCP/IP sockets in C practical guide for programmers.
8.	Anggoro, W. (2015). Boost.Asio C++ network programming: learn effective C++ network programming with Boost.Asio and become a proficient C++ network programmer.
9.	Ferguson, B. (2005). Network+ fast pass. Chapter 2.2. Identifying the Seven Layers of the OSI Model and Their Functions.
10.	IBM Cloud Education (2021). The Fundamentals of Netwroking. IBM Cloud Learn Hub. https://www.ibm.com/uk-en/cloud/learn/networking-a-complete-guide
11.	Savvy Tower (2020). The Different Types of Network Topology. Savvy Tower. https://savvytower.com/types-of-network-topology/
12.	Hasson, E. (2020). OSI Model. Imperva. https://www.imperva.com/learn/application-security/osi-model/
13.	Bhardwaj, R. (2020). Socket vs Port – Detailed Explanation and Difference. IP With Ease. https://ipwithease.com/ports-and-socket-explanation/
14.	Liang, H.; Hasan, M. et. Al. (2022). Understand TCP/IP addressing and subnetting basics. Microsoft Docs. https://docs.microsoft.com/en-us/troubleshoot/windows-client/networking/tcpip-addressing-and-subnetting
15.	Bischoff, P. (2019). UDP vs TCP: What are they and how do they differ? Comparitech. https://www.comparitech.com/blog/vpn-privacy/udp-vs-tcp-ip/
16.	Bshokati (2013). Simple client-server network using C++ and Windows Winsock. Code Project. https://www.codeproject.com/Articles/412511/Simple-client-server-network-using-Cplusplus-and-W
17.	Gyrling, C. (2015). Parallelizing the Naughty Dog Engine Using Fibers. https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
18.	Guided Hacking. (2019). C++ Winsock Networking Tutorial – Introduction. https://www.youtube.com/watch?v=GiOgWfmWzPY
19.	The Cherno (2021). CHESS! // Code Review. https://www.youtube.com/watch?v=WKs685H6uOQ
20.	Lanthier, M. (2017). Chapter 12 - Network Programming. Teaching notes from School of Computer Science at Carleton University in Ottawa, Ontario. https://people.scs.carleton.ca/~lanthier/teaching/COMP1406/Notes/COMP1406_Ch12_NetworkProgramming.pdf

