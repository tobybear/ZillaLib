/*
  ZillaLib
  Copyright (C) 2010-2020 Bernhard Schelling

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

#ifdef ZL_USE_NETWORK

#if defined(__wasm__) || defined(__EMSCRIPTEN__) || defined(__native_client__)
#define ZL_NO_SOCKETS
#endif

struct ZL_Network
{
	static bool Init();
	static void DeInit();
};

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
	void* data;
	ZL_PeerHandle handle;
};

struct ZL_Server
{
	ZL_Server();
	ZL_Server(int server_port, unsigned short max_connections, const char *bind_host = NULL, unsigned char num_channels = 1);
	ZL_Server(const char *p2prelay_host, int p2prelay_port, unsigned short max_connections = 2, const void* relaykey = NULL, unsigned char relaykey_length = 0, unsigned char num_channels = 1);
	~ZL_Server();
	ZL_Server(const ZL_Server &source);
	ZL_Server &operator =(const ZL_Server &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Server &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Server &b) const { return (impl!=b.impl); }

	void Open(int server_port, unsigned short max_connections, const char *bind_host = NULL, unsigned char num_channels = 1);
	void OpenP2P(const char *p2prelay_host, int p2prelay_port, unsigned short max_connections = 2, const void* relaykey = NULL, unsigned char relaykey_length = 0, unsigned char num_channels = 1);
	void Close(unsigned int closemsg = 0);

	//Send to a single client
	void Send(ZL_PeerHandle handle, const void* data, size_t length, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0);
	void Send(ZL_PeerHandle handle, const ZL_Packet& packet)                                                                                           { Send(handle, packet.data, packet.length, packet.type, packet.channel); }
	void Send(ZL_PeerHandle handle, const char *data, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                      { Send(handle, data, (data ? strlen(data) : 0), type, channel); }
	void Send(ZL_PeerHandle handle, const ZL_String& str, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                  { Send(handle, (str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), type, channel); }
	template <typename T> void Send(ZL_PeerHandle handle, const T& packet, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0) { Send(handle, &packet, sizeof(packet), type, channel); }

	//Send to all clients
	void Broadcast(const void* data, size_t length, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0);
	void Broadcast(const ZL_Packet& packet)                                                                                           { Broadcast(packet.data, packet.length, packet.type, packet.channel); }
	void Broadcast(const char *data, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                      { Broadcast(data, (data ? strlen(data) : 0), type, channel); }
	void Broadcast(const ZL_String& str, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                  { Broadcast((str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), type, channel); }
	template <typename T> void Broadcast(const T& packet, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0) { Broadcast(&packet, sizeof(packet), type, channel); }

	//Send to all clients except one
	void Broadcast(const void* data, size_t length, ZL_PeerHandle handle_except, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0);
	void Broadcast(const ZL_Packet& packet, ZL_PeerHandle handle_except)                                                                                           { Broadcast(packet.data, packet.length, handle_except, packet.type, packet.channel); }
	void Broadcast(const char *data, ZL_PeerHandle handle_except, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                      { Broadcast(data, (data ? strlen(data) : 0), handle_except, type, channel); }
	void Broadcast(const ZL_String& str, ZL_PeerHandle handle_except, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                  { Broadcast((str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), handle_except, type, channel); }
	template <typename T> void Broadcast(const T& packet, ZL_PeerHandle handle_except, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0) { Broadcast(&packet, sizeof(packet), handle_except, type, channel); }

	void DisconnectPeer(ZL_PeerHandle handle, unsigned int closemsg = 0);
	void GetPeerHandles(std::vector<ZL_PeerHandle>& out_list);
	void GetPeerDetails(std::vector<ZL_Peer>& out_list);
	void SetPeerData(ZL_PeerHandle handle, void* data);
	void SetPeerData(const ZL_Peer& peer, void* data);
	bool IsOpened();
	size_t GetPeerCount();
	bool IsP2PMaster();

	ZL_Signal_v1<const ZL_Peer&>& sigConnected();
	ZL_Signal_v2<const ZL_Peer&, unsigned int>& sigDisconnected();
	ZL_Signal_v2<const ZL_Peer&, ZL_Packet&>& sigReceived();

	private: struct ZL_Server_Impl* impl;
};

struct ZL_Client
{
public:
	ZL_Client();
	ZL_Client(const char *host, int port, unsigned char num_channels = 1);
	~ZL_Client();
	ZL_Client(const ZL_Client &source);
	ZL_Client &operator =(const ZL_Client &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Client &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Client &b) const { return (impl!=b.impl); }

	void Connect(const char *host, int port, unsigned char num_channels = 1);
	void Disconnect(unsigned int closemsg = 0);

	void Send(const void* data, size_t length, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0);
	void Send(const ZL_Packet& packet)                                                                                           { Send(packet.data, packet.length, packet.type, packet.channel); }
	void Send(const char *data, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                      { Send(data, (data ? strlen(data) : 0), type, channel); }
	void Send(const ZL_String& str, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0)                  { Send((str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), type, channel); }
	template <typename T> void Send(const T& packet, ZL_Packet_Reliability type = ZL_PACKET_RELIABLE, unsigned char channel = 0) { Send(&packet, sizeof(packet), type, channel); }

	bool IsConnected();
	ZL_Signal_v0& sigConnected();
	ZL_Signal_v1<unsigned int>& sigDisconnected();
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

struct ZL_WebSocketServer
{
	ZL_WebSocketServer();
	ZL_WebSocketServer(int server_port, unsigned short max_connections, const char *bind_host = NULL);
	~ZL_WebSocketServer();
	ZL_WebSocketServer(const ZL_WebSocketServer &source);
	ZL_WebSocketServer &operator =(const ZL_WebSocketServer &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_WebSocketServer &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_WebSocketServer &b) const { return (impl!=b.impl); }

	void Open(int server_port, unsigned short max_connections, const char *bind_host = NULL);
	void Close(short code = 0);

	//Send to one client
	void Send(ZL_PeerHandle handle, const void* data, size_t length, bool is_text = false);
	void Send(ZL_PeerHandle handle, const char *data, bool is_text = true)     { Send(handle, data, (data ? strlen(data) : 0), is_text); }
	void Send(ZL_PeerHandle handle, const ZL_String& str, bool is_text = true) { Send(handle, (str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), is_text); }
	template <typename T> void Send(ZL_PeerHandle handle, const T& packet)     { Send(handle, &packet, sizeof(packet), false); }

	//Send to all clients
	void Broadcast(const void* data, size_t length, bool is_text = false);
	void Broadcast(const char *data, bool is_text = true)     { Broadcast(data, (data ? strlen(data) : 0), is_text); }
	void Broadcast(const ZL_String& str, bool is_text = true) { Broadcast((str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), is_text); }
	template <typename T> void Broadcast(const T& packet)     { Broadcast(&packet, sizeof(packet), false); }

	//Send to all clients except one
	void Broadcast(const void* data, size_t length, ZL_PeerHandle handle_except, bool is_text = false);
	void Broadcast(const char *data, ZL_PeerHandle handle_except, bool is_text = true)     { Broadcast(data, (data ? strlen(data) : 0), handle_except, is_text); }
	void Broadcast(const ZL_String& str, ZL_PeerHandle handle_except, bool is_text = true) { Broadcast((str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), handle_except, is_text); }
	template <typename T> void Broadcast(const T& packet, ZL_PeerHandle handle_except)     { Broadcast(&packet, sizeof(packet), handle_except, false); }

	void DisconnectPeer(ZL_PeerHandle handle, short code = 0);
	void GetPeerHandles(std::vector<ZL_PeerHandle>& out_list);
	void GetPeerDetails(std::vector<ZL_Peer>& out_list);
	void SetPeerData(ZL_PeerHandle handle, void* data);
	void SetPeerData(const ZL_Peer& peer, void* data);
	bool IsOpened();
	size_t GetPeerCount();
	
	ZL_Signal_v2<const ZL_Peer&, const ZL_String&>& sigConnected();
	ZL_Signal_v2<const ZL_Peer&, unsigned short>& sigDisconnected();
	ZL_Signal_v2<const ZL_Peer&, const ZL_String&>& sigReceivedText();
	ZL_Signal_v3<const ZL_Peer&, const void*, size_t>& sigReceivedBinary();

	private: struct ZL_WebSocketServer_Impl* impl;
};

#endif //ZL_NO_SOCKETS

struct ZL_WebSocketClient
{
	ZL_WebSocketClient();
	ZL_WebSocketClient(const char *url);
	~ZL_WebSocketClient();
	ZL_WebSocketClient(const ZL_WebSocketClient &source);
	ZL_WebSocketClient &operator =(const ZL_WebSocketClient &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_WebSocketClient &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_WebSocketClient &b) const { return (impl!=b.impl); }

	void Connect(const char *url);
	void Disconnect(unsigned short code = 1000, const char *reason = NULL, size_t reason_length = 0) const;
	bool IsConnected() const;

	void Send(const void* data, size_t length, bool is_text = false);
	void Send(const char *data, bool is_text = true)     { Send(data, (data ? strlen(data) : 0), is_text); }
	void Send(const ZL_String& str, bool is_text = true) { Send((str.empty() ? 0 : &*str.begin()), (str.empty() ? 0 : str.size()), is_text); }
	template <typename T> void Send(const T& packet)     { Send(&packet, sizeof(packet), false); }

	ZL_Signal_v0& sigConnected();
	ZL_Signal_v1<unsigned short>& sigDisconnected();
	ZL_Signal_v1<const ZL_String&>& sigReceivedText();
	ZL_Signal_v2<const void*, size_t>& sigReceivedBinary();

	private: struct ZL_WebSocketClient_Impl* impl;
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
	ZL_HttpConnection& SetPostData(const char *data); //zero terminated string
	ZL_HttpConnection& SetPostData(const void* data, size_t length);
	ZL_HttpConnection& ClearPostData();

	//Stream data in packets with a final zero length terminating packet
	ZL_HttpConnection& SetDoStreamData(bool DoStreamData = true);

	//set custom timeout for http request (default 10 seconds)
	ZL_HttpConnection& SetTimeout(unsigned int timeout_msec = 10000);

	void Connect(const char *url);

	ZL_Signal_v2<int, const ZL_String&>& sigReceivedString();
	ZL_Signal_v3<int, const char*, size_t>& sigReceivedData();

	private: struct ZL_HttpConnection_Impl* impl;
};

#endif

#endif //__ZL_NETWORK__
