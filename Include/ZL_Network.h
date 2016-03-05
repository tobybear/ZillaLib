/*
  ZillaLib
  Copyright (C) 2010-2016 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __ZL_NETWORK__
#define __ZL_NETWORK__

#include "ZL_Signal.h"
#include "ZL_String.h"

#if defined(__native_client__) || defined(__EMSCRIPTEN__)
#define ZL_NO_SOCKETS
#endif

#ifndef ZL_NO_SOCKETS

#include <vector>

#if (defined(_MSC_VER))
#pragma comment (lib, "ws2_32.lib")
#endif

enum ZL_Packet_Reliability
{
	ZL_PACKET_RELIABLE = 0,
	ZL_PACKET_UNRELIABLE = 1,
	ZL_PACKET_UNSEQUENCED = 2,
	ZL_PACKET_ERROR = 3
};

struct ZL_Packet
{
	void* data;
	size_t length;
	unsigned char channel;
	ZL_Packet_Reliability type;
};

typedef void* ZL_PeerHandle;

struct ZL_Peer
{
	unsigned int host;
	unsigned short port;
	void** data;
	ZL_PeerHandle handle;
};

struct ZL_Server
{
	ZL_Server();
	ZL_Server(int server_port, int num_connections, const char *bind_host = NULL);
	~ZL_Server();
	ZL_Server(const ZL_Server &source);
	ZL_Server &operator =(const ZL_Server &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Server &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Server &b) const { return (impl!=b.impl); }

	void Open(int server_port, int num_connections, const char *bind_host = NULL);
	void Close(unsigned int closemsg = 0);
	void Send(std::vector<ZL_PeerHandle> peerhandles, ZL_Packet &packet);
	void Send(ZL_PeerHandle peerhandle, ZL_Packet &packet);
	void Broadcast(ZL_Packet &packet);
	void Broadcast(ZL_Packet &packet, ZL_PeerHandle peerhandle_except);
	void Send(std::vector<ZL_PeerHandle> peerhandles, const void* data, size_t length, unsigned char channel = 0, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE);
	void Send(ZL_PeerHandle peerhandle, const void* data, size_t length, unsigned char channel = 0, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE);
	void Broadcast(const void* data, size_t length, unsigned char channel = 0, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE);
	void Broadcast(const void* data, size_t length, ZL_PeerHandle peerhandle_except, unsigned char channel = 0, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE);
	const std::vector<ZL_PeerHandle> &GetPeerHandles();
	std::vector<ZL_Peer> GetPeerDetails();
	bool IsOpened();

	ZL_Signal_v1<ZL_Peer&>& sigConnected();
	ZL_Signal_v1<ZL_Peer&>& sigDisconnected();
	ZL_Signal_v2<ZL_Peer&, ZL_Packet&>& sigReceived();

	private: struct ZL_Server_Impl* impl;
};

struct ZL_Client
{
public:
	ZL_Client();
	ZL_Client(const char *host, int port, int num_channels = 1);
	~ZL_Client();
	ZL_Client(const ZL_Client &source);
	ZL_Client &operator =(const ZL_Client &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Client &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Client &b) const { return (impl!=b.impl); }

	void Connect(const char *host, int port, int num_channels = 1);
	void NatPunch(const char *relay_host, int relay_port, const unsigned char punch_key[8], int num_channels = 1);
	void Disconnect(unsigned int closemsg = 0);
	void Send(ZL_Packet &packet);
	void Send(const void* data, size_t length, unsigned char channel = 0, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE);
	bool IsConnected();
	bool IsNatPunchSlave();

	ZL_Signal_v0& sigConnected();
	ZL_Signal_v0& sigDisconnected();
	ZL_Signal_v1<ZL_Packet&>& sigReceived();

	private: struct ZL_Client_Impl* impl;
};

struct ZL_RawSocket
{
	enum Type { TYPE_TCP, TYPE_UDP };
	ZL_RawSocket();
	~ZL_RawSocket();
	ZL_RawSocket(const ZL_RawSocket &source);
	ZL_RawSocket &operator =(const ZL_RawSocket &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_RawSocket &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_RawSocket &b) const { return (impl!=b.impl); }

	bool ConnectSync(const char *host, int port, Type type = TYPE_TCP);
	void Disconnect();
	int Send(const void* data, size_t length);
	bool IsConnected();

	ZL_Signal_v1<ZL_Packet&>& sigReceived() const;

	private: struct ZL_RawSocket_Impl* impl;
};

#endif //ZL_NO_SOCKETS

struct ZL_Network
{
	static bool Init();
	static void DeInit();
};

struct ZL_HttpConnection
{
	ZL_HttpConnection();
	ZL_HttpConnection(const char *url);
	~ZL_HttpConnection();
	ZL_HttpConnection(const ZL_HttpConnection &source);
	ZL_HttpConnection &operator =(const ZL_HttpConnection &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_HttpConnection &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_HttpConnection &b) const { return (impl!=b.impl); }

	ZL_HttpConnection& SetURL(const char *url);
	ZL_HttpConnection& SetPostData(const char *data); //zero terminated string
	ZL_HttpConnection& SetPostData(const void* data, size_t length);

	//Stream data in packets with a final zero length terminating packet
	ZL_HttpConnection& SetDoStreamData(bool DoStreamData = true);

	#ifndef ZL_NO_SOCKETS
	//set custom timeout for http request, most platforms use a appropriate default
	ZL_HttpConnection& SetTimeout(unsigned int timeout_msec = 10000);
	#endif

	void Connect() const;

	ZL_Signal_v2<int, const ZL_String&>& sigReceivedString();
	ZL_Signal_v3<int, const char*, size_t>& sigReceivedData();

	private: struct ZL_HttpConnection_Impl* impl;
};

#endif //__ZL_NETWORK__
