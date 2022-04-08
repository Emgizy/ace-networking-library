# ACE 
## *C++ Networking Library for Windows*

![ACE-logos - Copy](https://user-images.githubusercontent.com/48070316/162492398-7d98083e-c759-4ca9-8bbe-b3886d7fb156.jpeg)

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
_WIP_

## *Client and Server Loops*
_WIP_

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
Included with this library are two demos which replicate a messaging chat and a Pong clone so that you can test them out for yourself.

As this project goes on further, more demos will be created and uploaded for use and inspiration.

### Chat
A simple chat executable, where clients can connect to a server and send messages to one another.

This project was created to iron out the simplest bugs in the library and was used to test its speed against other libraries. 
![Chat](https://user-images.githubusercontent.com/48070316/162490711-6957dc8c-b428-46e5-9204-089c173cc10c.png)

### Pong
An online recreation of the classic 1972 Atari arcade game; Pong.

This project was centred around testing if the library’s design was actually usable for a game deliverable and tried to expose any design flaws that the library could have, as well as where it can be improved in the future.
![Pong](https://user-images.githubusercontent.com/48070316/162490534-f90b9732-745c-40f3-b5b0-4ed24386a077.png)

## *Acknowledgements*

Thanks to Luke Melville and Paul Parry for their unbelievable help in understanding the topic at hand.

Also thanks to SFML for their networking API from which I took some valuable inspiration when designing ACE.
