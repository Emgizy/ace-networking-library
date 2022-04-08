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
There are three main classes to understand in this library: the Server, the Client and the Packet.

To create a server, the code might seem difficult, but trust me, its simpler than it seems.

```cpp
  Server server;
```

What? You think that's difficult? We might have to talk a bit about your programming experience then...

To create and open a server it is as simple as instantiating a server class. This server will be opened at the DEFAULT_PORT specified on the top of the "NetworkingLibrary.h" file as a non-blocking TCP socket (all of you networking geniuses know what I'm talking about).

This server has the option to accept clients, remove them, send data to one client in particular or to all of them.





## *Client and Server Loops*

As the library stands right now, the server is created whenever a "Server" class is created, and destroyed when it's destroyed. The same goes for clients, as they try to connect to the server once when they are created, and will throw an exception if this fails. This will be improved in the future, but, for now, we will use the tools we have available.

To handle this, as well as how packets are received by both the client and the server, there is a custom "ClientGame" and "ServerGame" classes that serve as interfaces for basic functions the application may have. Each has a copy of either a Client or a Server, and will take care of most of the networking stuff for you. The only things left for you are to override the "HandleSinglePacket" function which will receive a buffer of bytes (in this case, a char*) and decide what to do with it.

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

If your packets have more data than the single "flag" of their type, then you can get it by deserializing them. This will be further explained in the [Custom Packets](#-*Custom-Packets*) section.

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

Then, to send a custom packet it's as easy as

```cpp
MessagePacket msg;
msg.m_Message = "Hello World!";
m_Client->SendPacketToServer(&msg);
```

And to handle receving it, just add another case to your HandleSinglePacket switch statement
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

## *Acknowledgements*

Thanks to Luke Melville and Paul Parry for their unbelievable help in understanding the topic at hand.

Also thanks to SFML for their networking API from which I took some valuable inspiration when designing ACE.
