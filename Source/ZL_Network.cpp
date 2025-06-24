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

#ifdef ZL_USE_NETWORK

#include "ZL_Network.h"
#include "ZL_Application.h"
#include "ZL_Impl.h"
#include "ZL_Platform.h"

#ifdef ZL_USE_ENET

#include "ZL_Data.h"
#include "enet/enet.h"
#include <vector>

static std::vector<struct ZL_Connection_Impl*> *OpenConnections = NULL;
static ZL_MutexHandle ZL_OpenConnectionsMutex;
static void _ZL_Network_KeepAlive();

struct ZL_Connection_Impl : ZL_Impl
{
	virtual bool KeepAlive() = 0;
	virtual void ShutDown() = 0;
	bool connectionWasClosed;
	
protected:
	~ZL_Connection_Impl()
	{
		ZL_MutexLock(ZL_OpenConnectionsMutex);
		for (std::vector<ZL_Connection_Impl*>::iterator it = OpenConnections->begin(); it != OpenConnections->end(); ++it)
			if (*it == this) { OpenConnections->erase(it); break; }
		ZL_MutexUnlock(ZL_OpenConnectionsMutex);
	}

	void OpenConnectionsAdd()
	{
		ZL_ASSERTMSG(OpenConnections, "ZL_Network::Init needs to be called before using network functions");
		connectionWasClosed = false;
		ZL_MutexLock(ZL_OpenConnectionsMutex);
		for (std::vector<ZL_Connection_Impl*>::iterator it = OpenConnections->begin(); it != OpenConnections->end(); ++it)
			if (*it == this) goto skipadd;
		OpenConnections->push_back(this);
		skipadd:
		ZL_MutexUnlock(ZL_OpenConnectionsMutex);
	}
};

static void _ZL_Network_KeepAlive()
{
	ZL_MutexLock(ZL_OpenConnectionsMutex);
	for (std::vector<ZL_Connection_Impl*>::iterator it = OpenConnections->begin(); it != OpenConnections->end();)
	{
		if ((*it)->connectionWasClosed || !(*it)->KeepAlive())
			it = OpenConnections->erase(it);
		else
			++it;
	}
	ZL_MutexUnlock(ZL_OpenConnectionsMutex);
}

bool ZL_Network::Init()
{
	if (OpenConnections) return true;
	if (enet_initialize() != 0) return false;
	OpenConnections = new std::vector<ZL_Connection_Impl*>();
	ZL_MutexInit(ZL_OpenConnectionsMutex);
	ZL_Application::sigKeepAlive.connect(&_ZL_Network_KeepAlive);
	return true;
}

void ZL_Network::DeInit()
{
	if (!OpenConnections) return;
	ZL_MutexLock(ZL_OpenConnectionsMutex);
	for (std::vector<ZL_Connection_Impl*>::iterator it = OpenConnections->begin(); it != OpenConnections->end(); ++it)
	{
		(*it)->ShutDown();
	}
	OpenConnections->clear();
	delete OpenConnections;
	OpenConnections = NULL;
	ZL_MutexUnlock(ZL_OpenConnectionsMutex);
	ZL_MutexDestroy(ZL_OpenConnectionsMutex);
}

static char* GetHTTPContentStart(char* buffer, size_t length)
{
	for (char *content = buffer, *end = buffer + length; content <= end-2; content++)
		if ((content[0]=='\n'&&content[1]=='\n') || (content <= end-4 && content[0]=='\r'&&content[1]=='\n'&&content[2]=='\r'&&content[3]=='\n'))
			return content + (content[0]=='\n' ? 2 : 4);
	return NULL;
}

//----------------------------------------------------------------------------------------------------------------------------------------------

struct ZL_Server_Impl : ZL_Connection_Impl
{
	ENetHost *host;
	struct sP2P { bool (*doKeepAlive)(ZL_Server_Impl*); void (*doClose)(ZL_Server_Impl*); ENetAddress relay_server; unsigned char relaykey[0xFF], relaykey_length, is_master; ticks_t time_open; } *P2P;
	ZL_Signal_v1<const ZL_Peer&> sigConnected;
	ZL_Signal_v2<const ZL_Peer&, unsigned int> sigDisconnected;
	ZL_Signal_v2<const ZL_Peer&, ZL_Packet&> sigReceived;

	ZL_Server_Impl() : host(NULL), P2P(NULL) { }

	void Open(int server_port, unsigned short num_connections, const char *bind_host, unsigned char num_channels)
	{
		ZL_ASSERTMSG(OpenConnections, "ZL_Network::Init needs to be called before using network functions");
		if (host) Close(0);

		ENetAddress address;
		if (bind_host) enet_address_set_host(&address, bind_host);
		else address.host = ENET_HOST_ANY;
		address.port = server_port;

		host = enet_host_create(&address, num_connections, num_channels, 0, 0);
		if (!host) return;
		OpenConnectionsAdd();
	}
	
	void OpenP2P(const char *p2prelay_host, int p2prelay_port, unsigned short num_connections, const void* relaykey, unsigned char relaykey_length, unsigned char num_channels)
	{
		ZL_ASSERTMSG(OpenConnections, "ZL_Network::Init needs to be called before using network functions");
		if (host) Close(0);

		host = enet_host_create(NULL, num_connections, num_channels, 0, 0);
		if (!host) return;

		P2P = new sP2P();
		P2P->doKeepAlive = &KeepAliveP2P;
		P2P->doClose = &CloseP2P;
		enet_address_set_host(&P2P->relay_server, p2prelay_host);
		P2P->relay_server.port = p2prelay_port;
		P2P->is_master = 0xFF;
		P2P->time_open = ZLTICKS;
		memcpy(P2P->relaykey, relaykey, P2P->relaykey_length = relaykey_length);

		unsigned char buffer[1 + 2 + 0xFF]; //command + open_connections + key
		ENetBuffer bs;
		bs.data = (void*)buffer;
		buffer[0] = 100; //command 100 = new connection
		buffer[1] = (num_connections - 1) & 0xFF;
		buffer[2] = (num_connections - 1) >> 8;
		memcpy(&buffer[1 + 2], relaykey, relaykey_length);
		bs.dataLength = 1 + 2 + relaykey_length;
		enet_socket_send(host->socket, &P2P->relay_server, &bs, 1);

		OpenConnectionsAdd();
	}

	void Close(unsigned int closemsg)
	{
		if (!host) return;
		if (P2P && P2P->doClose) P2P->doClose(this);
		for (ENetPeer* it = host->peers; it != &host->peers[host->peerCount]; ++it)
			if (it->state == ENET_PEER_STATE_CONNECTED)
				enet_peer_disconnect(it, closemsg);
		enet_host_flush(host);
		enet_host_destroy(host);
		host = NULL;
		connectionWasClosed = true;
	}

protected:
	~ZL_Server_Impl() { Close(0); }
	virtual void ShutDown() { Close(0); }

	static void CloseP2P(ZL_Server_Impl* self)
	{
		if (self->P2P->is_master == 1)
		{
			unsigned char buffer[1 + 0xFF]; //command + key
			ENetBuffer bs;
			bs.data = (void*)buffer;
			buffer[0] = 101; //command 101 = remove active relay
			memcpy(&buffer[1], self->P2P->relaykey, self->P2P->relaykey_length);
			bs.dataLength = 1 + self->P2P->relaykey_length;
			enet_socket_send(self->host->socket, &self->P2P->relay_server, &bs, 1);
			unsigned int waitCondition = ENET_SOCKET_WAIT_RECEIVE;
			enet_socket_wait(self->host->socket, &waitCondition, 100);
		}
		delete self->P2P;
		self->P2P = NULL;
	}

	virtual bool KeepAlive()
	{
		if (!ZL_VERIFY(host)) return false;
		if (P2P && P2P->doKeepAlive && P2P->doKeepAlive(this)) return true;
		ENetEvent event;
		while (enet_host_service(host, &event, 0) > 0)
		{
			ZL_Peer peer = { ENET_NET_TO_HOST_32(event.peer->address.host), event.peer->address.port, event.peer->data, event.peer };
			switch (event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
					{
						ZL_Packet packet = { event.packet->data, event.packet->dataLength, event.channelID, (event.packet->flags & ENET_PACKET_FLAG_RELIABLE ? ZL_PACKET_RELIABLE : (event.packet->flags & ENET_PACKET_FLAG_UNSEQUENCED ? ZL_PACKET_UNSEQUENCED : ZL_PACKET_UNRELIABLE)) };
						sigReceived.call(peer, packet);
					}
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_CONNECT:
					sigConnected.call(peer); //ToDo: include event.data
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					sigDisconnected.call(peer, event.data);
					break;
				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}
		return true;
	}

	static bool KeepAliveP2P(ZL_Server_Impl* self)
	{
		unsigned int waitCondition = ENET_SOCKET_WAIT_RECEIVE;
		if (enet_socket_wait(self->host->socket, &waitCondition, 0) || !(waitCondition & ENET_SOCKET_WAIT_RECEIVE))
		{
			if (ZLSINCE(self->P2P->time_open) >= ENET_PEER_TIMEOUT_MAXIMUM)
			{
				ZL_Peer peer = { ENET_NET_TO_HOST_32(self->P2P->relay_server.host), self->P2P->relay_server.port, NULL, NULL };
				self->sigDisconnected.call(peer, 0);
				self->P2P->doKeepAlive = NULL;
				return false;
			}
			return true;
		}

		unsigned char buffer[1 + 4 + 2 + 0xFF]; //command + [ip + port] + key
		ENetBuffer br;
		br.data = buffer;
		br.dataLength = sizeof(buffer);
		int rec = enet_socket_receive(self->host->socket, NULL, &br, 1);
		if (rec < 1 || rec > (int)sizeof(buffer) || buffer[0] < 200 || buffer[0] > 201) return true; //unknown response
		if (buffer[0] == 200) //command 200 = become host
		{
			if (memcmp(buffer + (1), self->host->packetData, rec - (1))) return true; //key mismatch
			self->P2P->is_master = 1;
			ZL_LOG3("NET", "    Received nat punch become host command: [%.*s] (%d bytes)", rec - (1), buffer + (1), rec);
		}
		else if (buffer[0] == 201) //command 201 = become guest
		{
			if (rec < (1 + 4 + 2) || memcmp(buffer + (1 + 4 + 2), self->host->packetData, rec - (1 + 4 + 2))) return true; //key mismatch
			ENetAddress address;
			address.host = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);
			address.port = buffer[5] | (buffer[6] << 8);
			self->P2P->is_master = 0;
			//self->host->peers->outgoingSessionID = 0; //change from 0xFF to 0 to make outgoingSessionID to 1 on first connection (incomingSessionID will be 1 on master)
			ZL_LOG3("NET", "    Received nat punch guest info - connectID: %d - Host: %X - Port: %d", self->host->peers->connectID, address.host, address.port);
			enet_host_connect(self->host, &address, self->host->channelLimit, 0);
		}
		self->P2P->doKeepAlive = NULL;
		return false;
	}
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Server)

ZL_Server::ZL_Server(int server_port, unsigned short max_connections, const char *bind_host, unsigned char num_channels) : impl(new ZL_Server_Impl())
{
	impl->Open(server_port, max_connections, bind_host, num_channels);
}

ZL_Server::ZL_Server(const char *p2prelay_host, int p2prelay_port, unsigned short max_connections, const void* relaykey, unsigned char relaykey_length, unsigned char num_channels) : impl(new ZL_Server_Impl())
{
	impl->OpenP2P(p2prelay_host, p2prelay_port, max_connections, relaykey, relaykey_length, num_channels);
}

void ZL_Server::Open(int server_port, unsigned short max_connections, const char *bind_host, unsigned char num_channels)
{
	if (!impl) impl = new ZL_Server_Impl();
	impl->Open(server_port, max_connections, bind_host, num_channels);
}

void ZL_Server::OpenP2P(const char *p2prelay_host, int p2prelay_port, unsigned short max_connections, const void* relaykey, unsigned char relaykey_length, unsigned char num_channels)
{
	if (!impl) impl = new ZL_Server_Impl();
	impl->OpenP2P(p2prelay_host, p2prelay_port, max_connections, relaykey, relaykey_length, num_channels);
}

void ZL_Server::Close(unsigned int closemsg)
{
	if (impl && impl->host) impl->Close(closemsg);
}

void ZL_Server::Send(ZL_PeerHandle peer, const void* data, size_t length, ZL_Packet_Reliability type, unsigned char channel)
{
	if (impl && impl->host)
		enet_peer_send((ENetPeer*)peer, channel, enet_packet_create(data, length, (type == ZL_PACKET_RELIABLE ? ENET_PACKET_FLAG_RELIABLE : (type == ZL_PACKET_UNRELIABLE ? 0 : ENET_PACKET_FLAG_UNSEQUENCED))));
}

void ZL_Server::Broadcast(const void* data, size_t length, ZL_Packet_Reliability type, unsigned char channel)
{
	if (impl && impl->host)
		enet_host_broadcast(impl->host, channel, enet_packet_create(data, length, (type == ZL_PACKET_RELIABLE ? ENET_PACKET_FLAG_RELIABLE : (type == ZL_PACKET_UNRELIABLE ? 0 : ENET_PACKET_FLAG_UNSEQUENCED))));
}

void ZL_Server::Broadcast(const void* data, size_t length, ZL_PeerHandle peer_except, ZL_Packet_Reliability type, unsigned char channel)
{
	if (!impl || !impl->host) return;
	ENetPacket* enetpacket = enet_packet_create(data, length, (type == ZL_PACKET_RELIABLE ? ENET_PACKET_FLAG_RELIABLE : (type == ZL_PACKET_UNRELIABLE ? 0 : ENET_PACKET_FLAG_UNSEQUENCED)));
	for (ENetPeer *it = impl->host->peers, *end = &impl->host->peers[impl->host->peerCount]; it != end; ++it)
		if (it->state == ENET_PEER_STATE_CONNECTED && it != peer_except)
			enet_peer_send(it, channel, enetpacket);
	if (enetpacket->referenceCount == 0) enet_packet_destroy(enetpacket);
}

void ZL_Server::DisconnectPeer(ZL_PeerHandle peerhandle, unsigned int closemsg)
{
	if (impl && impl->host) for (ENetPeer *it = impl->host->peers, *end = &impl->host->peers[impl->host->peerCount]; it != end; ++it)
	{
		if (it != peerhandle) continue;
		if (it->state == ENET_PEER_STATE_CONNECTED) enet_peer_disconnect(it, closemsg);
		return;
	}
}

void ZL_Server::GetPeerHandles(std::vector<ZL_PeerHandle>& out_list)
{
	out_list.clear();
	if (impl && impl->host) for (ENetPeer *it = impl->host->peers, *end = &impl->host->peers[impl->host->peerCount]; it != end; ++it)
		if (it->state == ENET_PEER_STATE_CONNECTED)
			out_list.push_back(&*it);
}

void ZL_Server::GetPeerDetails(std::vector<ZL_Peer>& out_list)
{
	out_list.clear();
	if (impl && impl->host) for (ENetPeer *it = impl->host->peers, *end = &impl->host->peers[impl->host->peerCount]; it != end; ++it)
	{
		if (it->state != ENET_PEER_STATE_CONNECTED) continue;
		ZL_Peer peer = { ENET_NET_TO_HOST_32(it->address.host), it->address.port, it->data, it };
		out_list.push_back(peer);
	}
}

void ZL_Server::SetPeerData(ZL_PeerHandle handle, void* data)
{
	if (handle) ((ENetPeer*)handle)->data = data;
}

void ZL_Server::SetPeerData(const ZL_Peer& peer, void* data)
{
	((ENetPeer*)peer.handle)->data = data;
	((ZL_Peer*)&peer)->data = data;
}

bool ZL_Server::IsOpened()
{
	return (impl && impl->host);
}

size_t ZL_Server::GetPeerCount()
{
	size_t count = 0;
	if (impl && impl->host) for (ENetPeer *it = impl->host->peers, *end = &impl->host->peers[impl->host->peerCount]; it != end; ++it)
		if (it->state == ENET_PEER_STATE_CONNECTED) count++;
	return count;
}

bool ZL_Server::IsP2PMaster()
{
	return (impl && impl->P2P && impl->P2P->is_master == 1);
}

ZL_Signal_v1<const ZL_Peer&>&                ZL_Server::sigConnected()    { if (!impl) impl = new ZL_Server_Impl(); return impl->sigConnected;    }
ZL_Signal_v2<const ZL_Peer&, unsigned int >& ZL_Server::sigDisconnected() { if (!impl) impl = new ZL_Server_Impl(); return impl->sigDisconnected; }
ZL_Signal_v2<const ZL_Peer&, ZL_Packet&>&    ZL_Server::sigReceived()     { if (!impl) impl = new ZL_Server_Impl(); return impl->sigReceived;     }

//----------------------------------------------------------------------------------------------------------------------------------------------

struct ZL_Client_Impl : ZL_Connection_Impl
{
	ENetHost *host;
	ZL_Signal_v0 sigConnected;
	ZL_Signal_v1<unsigned int> sigDisconnected;
	ZL_Signal_v1<ZL_Packet&> sigReceived;

	ZL_Client_Impl() : host(NULL) { }

	void Connect(const char *connect_host, int port, unsigned char num_channels)
	{
		ZL_ASSERTMSG(OpenConnections, "ZL_Network::Init needs to be called before using network functions");
		if (host) Disconnect(0);

		host = enet_host_create(NULL, 1, num_channels, 0, 0);
		if (!host) return;

		ENetAddress address;
		enet_address_set_host(&address, connect_host);
		address.port = port;

		enet_host_connect(host, &address, num_channels, 0);

		OpenConnectionsAdd();
	}

	void Disconnect(unsigned int closemsg)
	{
		if (!host) return;
		if (host && host->peerCount) enet_peer_disconnect(host->peers, closemsg);
		if (host) enet_host_flush(host);
		enet_host_destroy(host);
		host = NULL;
		connectionWasClosed = true;
	}

protected:
	~ZL_Client_Impl() { Disconnect(0); }
	virtual void ShutDown() { Disconnect(0); }

	virtual bool KeepAlive()
	{
		if (!ZL_VERIFY(host)) return false;
		ENetEvent event;
		while (enet_host_service(host, &event, 0) > 0)
		{
			switch (event.type)
			{
				case ENET_EVENT_TYPE_RECEIVE:
					{
						ZL_Packet packet = { event.packet->data, event.packet->dataLength, event.channelID, (event.packet->flags & ENET_PACKET_FLAG_RELIABLE ? ZL_PACKET_RELIABLE : (event.packet->flags & ENET_PACKET_FLAG_UNSEQUENCED ? ZL_PACKET_UNSEQUENCED : ZL_PACKET_UNRELIABLE)) };
						sigReceived.call(packet);
					}
					enet_packet_destroy(event.packet);
					break;
				case ENET_EVENT_TYPE_CONNECT:
					sigConnected.call();
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					enet_host_destroy(host);
					host = NULL;
					sigDisconnected.call(event.data);
					return false;
				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}
		return true;
	}
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Client)

ZL_Client::ZL_Client(const char *host, int port, unsigned char num_channels) : impl(new ZL_Client_Impl())
{
	impl->Connect(host, port, num_channels);
}

void ZL_Client::Connect(const char *host, int port, unsigned char num_channels)
{
	if (!impl) impl = new ZL_Client_Impl();
	impl->Connect(host, port, num_channels);
}

void ZL_Client::Disconnect(unsigned int closemsg)
{
	if (impl) impl->Disconnect(closemsg);
}

void ZL_Client::Send(const void* data, size_t length, ZL_Packet_Reliability type, unsigned char channel)
{
	if (impl && impl->host && impl->host->peerCount)
		enet_peer_send(impl->host->peers, channel, enet_packet_create(data, length, (type == ZL_PACKET_RELIABLE ? ENET_PACKET_FLAG_RELIABLE : (type == ZL_PACKET_UNRELIABLE ? 0 : ENET_PACKET_FLAG_UNSEQUENCED))));
}

bool ZL_Client::IsConnected()
{
	return (impl && impl->host && impl->host->peerCount);
}

ZL_Signal_v0&               ZL_Client::sigConnected()    { if (!impl) impl = new ZL_Client_Impl(); return impl->sigConnected;    }
ZL_Signal_v1<unsigned int>& ZL_Client::sigDisconnected() { if (!impl) impl = new ZL_Client_Impl(); return impl->sigDisconnected; }
ZL_Signal_v1<ZL_Packet&>&   ZL_Client::sigReceived()     { if (!impl) impl = new ZL_Client_Impl(); return impl->sigReceived;     }

//----------------------------------------------------------------------------------------------------------------------------------------------

struct ZL_RawSocket_Impl : ZL_Connection_Impl
{
	ENetSocket socket;
	ZL_Signal_v1<ZL_Packet&> sigReceived;
	ZL_RawSocket::Type type;

	ZL_RawSocket_Impl() : socket(0) { }

	bool ConnectSync(const char *host, int port, ZL_RawSocket::Type type)
	{
		Close();
		this->type = type;
		ENetAddress address;
		if (enet_address_set_host(&address, host) != 0) return false;
		address.port = port;
		socket = enet_socket_create(type == ZL_RawSocket::TYPE_TCP ? ENET_SOCKET_TYPE_STREAM : ENET_SOCKET_TYPE_DATAGRAM);
		if (!socket) return false;
		if (enet_socket_connect(socket, &address) != 0) { enet_socket_destroy(socket); socket = 0; return false; }
		OpenConnectionsAdd();
		return true;
	}

	void Close()
	{
		if (socket) { enet_socket_destroy(socket); socket = 0; }
		connectionWasClosed = true;
	}

protected:
	~ZL_RawSocket_Impl() { Close(); }
	void ShutDown() { Close(); }

	virtual bool KeepAlive()
	{
		if (!ZL_VERIFY(socket)) return false;
		while (1)
		{
			unsigned int waitCondition = ENET_SOCKET_WAIT_RECEIVE;
			if (enet_socket_wait(socket, &waitCondition, 0) || !(waitCondition & ENET_SOCKET_WAIT_RECEIVE)) return true;

			char buffer[1024];
			ENetBuffer br;
			br.data = buffer;
			br.dataLength = 1024;
			int rec = enet_socket_receive(socket, NULL, &br, 1);
			//ZL_LOG1("NET", "    Received packet of size: %d", rec);
			ZL_Packet p;
			p.data = (rec>0 ? buffer : NULL);
			p.length = (rec>0 ? rec : 0);
			p.channel = 0;
			p.type = (rec < 0 ? ZL_PACKET_ERROR : ZL_PACKET_RELIABLE);
			sigReceived.call(p);
			if (type == ZL_RawSocket::TYPE_TCP && rec <= 0) return false;
		}
		return true;
	}
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_RawSocket)

bool ZL_RawSocket::ConnectSync(const char *host, int port, Type type)
{
	if (!impl) impl = new ZL_RawSocket_Impl();
	return impl->ConnectSync(host, port, type);
}

void ZL_RawSocket::Disconnect()
{
	if (!impl) return;
	impl->Close();
}

int ZL_RawSocket::Send(const void* data, size_t length)
{
	if (!impl) return 0;
	ENetBuffer bs;
	bs.data = (void*)data;
	bs.dataLength = length;
	return enet_socket_send(impl->socket, NULL, &bs, 1);
}

bool ZL_RawSocket::IsConnected() { return (impl && impl->socket != 0); }

ZL_Signal_v1<ZL_Packet&>& ZL_RawSocket::sigReceived() const
{
	return impl->sigReceived;
}

//----------------------------------------------------------------------------------------------------------------------------------------------

struct ZL_BasicTCPConnection_Impl : ZL_Connection_Impl
{
protected:
	ENetSocket socket;
	bool started, first_keep_alive, cancel, thread_active;

	ZL_BasicTCPConnection_Impl() : socket(0), started(false) { }
	~ZL_BasicTCPConnection_Impl() { Close(); }

	void StartConnection()
	{
		Close();
		cancel = false;
		started = thread_active = first_keep_alive = true;
		OpenConnectionsAdd();
		AddRef();
		ZL_CreateThread(&ConnectRunThread, (void*)this);
	}
	
	void Close()
	{
		if (!started) return;
		if (thread_active)
		{
			cancel = true;
			while (thread_active) { ZL_Delay(1); }
			cancel = false;
			DelRef();
		}
		if (socket)
		{
			enet_socket_destroy(socket);
			socket = 0;
		}
		started = false;
		connectionWasClosed = true;
	}

	virtual bool GetHost(ZL_String& host, int& port) = 0;

	static void* ConnectRunThread(void* vimpl)
	{
		ZL_BasicTCPConnection_Impl* impl = (ZL_BasicTCPConnection_Impl*)vimpl;
		ENetSocket tmpsocket = 0;
		ENetAddress address;
		ZL_String host; int port;
		bool aborted = !impl->GetHost(host, port);
		address.port = (unsigned short)port;

		//ZL_LOG2("NET", "Connect to host: %s - Port: %d", host.c_str(), port);
		if (impl->cancel || impl->GetRefCount() == 1 || aborted || enet_address_set_host(&address, host.c_str()) != 0) aborted = true;

		//ZL_LOG1("NET", "    Host resolved address: %x", address.host);
		else if (impl->cancel || impl->GetRefCount() == 1 || (!(tmpsocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM)))) aborted = true;

		//ZL_LOG0("NET", "    Socket created");
		else if (impl->cancel || impl->GetRefCount() == 1 || enet_socket_connect(tmpsocket, &address) != 0) aborted = true;

		if (aborted && tmpsocket) { enet_socket_destroy(tmpsocket); }
		else impl->socket = tmpsocket;
		impl->thread_active = false;
		return NULL;
	}

	static bool SplitUrl(const ZL_String& url, ZL_String::size_type protocol_name_length, ZL_String* out_path = NULL, ZL_String* out_host = NULL, int* out_port = NULL)
	{
		ZL_String::size_type host_begin = url.find("://");
		if (host_begin == ZL_String::npos || host_begin != protocol_name_length) return false;
		host_begin += 3;

		ZL_String::size_type host_end = url.find("/", host_begin);
		if (host_end == ZL_String::npos) host_end = url.length();

		ZL_String::size_type auth_split = url.find("@", host_begin);
		if (auth_split != ZL_String::npos && auth_split < host_end) { ZL_ASSERT(false); return false; } //http authentication not supported

		int port;
		ZL_String::size_type port_split = url.find(":", host_begin);
		if (port_split != ZL_String::npos && port_split > host_end) port_split = ZL_String::npos;
		port = (port_split != ZL_String::npos ? atoi(url.c_str()+port_split+1) : 80);
		if (port <= 0) return false; //port supplied was error

		if (out_port) *out_port = port;
		if (out_host) *out_host = url.substr(host_begin, (port_split == ZL_String::npos ? host_end : port_split) - host_begin);
		if (out_path) *out_path = (host_end == url.length() ? "/" : url.substr(host_end));
		return true;
	}

private:
	virtual void ShutDown() { Close(); }
};

struct ZL_HttpConnection_Impl : ZL_BasicTCPConnection_Impl
{
	ZL_String url;
	std::vector<char> post_data;
	ZL_Signal_v2<int, const ZL_String&> sigReceivedString;
	ZL_Signal_v3<int, const char*, size_t> sigReceivedData;
	std::vector<char> data;
	unsigned int timeout_tick, timeout_msec;
	bool dostream;
	int http_status;

	ZL_HttpConnection_Impl() : timeout_msec(10000), dostream(false), http_status(0) { }

	void Connect(const char* url)
	{
		this->url = url;
		StartConnection();
	}

protected:
	virtual bool GetHost(ZL_String& host, int& port)
	{
		return SplitUrl(url, 4, NULL, &host, &port); //only support "http" protocol
	}

	virtual bool KeepAlive()
	{
		if (first_keep_alive)
		{
			if (thread_active) return true;
			if (GetRefCount() == 1) { Close(); delete this; return false; }
			DelRef();
			if (socket)
			{
				ZL_String header, path, host;
				if (!SplitUrl(url, 4, &path, &host)) return false; //only support "http" protocol
				first_keep_alive = false;

				ENetBuffer bs;
				header << (post_data.size() ? "POST " : "GET ") << path << " HTTP/1.0\nHost: " << host << "\nUser-Agent: ZillaLib/1.0\n";
				if (post_data.size())
				{
					header << "Content-Length: " << ((unsigned int)post_data.size()) << "\n\n";
					size_t header_postdata_offset = header.size();
					header.resize(header_postdata_offset + post_data.size());
					memcpy(((char*)header.c_str())+header_postdata_offset, &post_data[0], post_data.size());
					bs.data = (void*)header.c_str();
					bs.dataLength = header.size();
				}
				else { header << "\n"; bs.data = (void*)header.c_str(); bs.dataLength = header.size(); }

				if (enet_socket_send(socket, NULL, &bs, 1) <= 0) return false;

				timeout_tick = (timeout_msec ? ZL_Application::Ticks + timeout_msec : 0);
				data.clear();
			}
		}
		if (!socket)
		{
			sigReceivedString.call(-1, ZL_String::EmptyString);
			sigReceivedData.call(-1, NULL, 0);
			Close();
			return false;
		}
		for (;;)
		{
			unsigned int waitCondition = ENET_SOCKET_WAIT_RECEIVE;
			if (enet_socket_wait(socket, &waitCondition, 0)) return true;
			if ((!(waitCondition & ENET_SOCKET_WAIT_RECEIVE)) && (!timeout_msec || ZL_Application::Ticks < timeout_tick)) return true;

			unsigned char buffer[1024];
			ENetBuffer br;
			br.data = buffer;
			br.dataLength = 1024;
			int rec = ((timeout_msec && ZL_Application::Ticks >= timeout_tick) ? 0 : enet_socket_receive(socket, NULL, &br, 1));
			//ZL_LOG1("NET", "    Received packet of size: %d", rec);
			if (dostream && http_status == 0)
			{
				char *status = NULL;
				for (status = (char*)buffer; status < (char*)buffer+rec-2; status++) if (*status == ' ') break;
				http_status = (status < (char*)buffer+rec-2 ? atoi(status+1) : 0);
			}
			if (rec > 0)
			{
				if (timeout_msec) timeout_tick = ZL_Application::Ticks + timeout_msec; //got data, update timeout tick
				if (dostream)
				{
					if (sigReceivedString.HasConnections())
						sigReceivedString.call(http_status, ZL_String((char*)buffer, rec));
					if (sigReceivedData.HasConnections())
						sigReceivedData.call(http_status, (char*)buffer, rec);
				}
				else
				{
					//store data until finished
					data.resize(data.size()+rec);
					memcpy(&data[data.size()-rec], buffer, rec);
				}
			}
			else
			{
				const char* content = NULL, *end = (data.empty() ? NULL : &data[0]+data.size()), *status = NULL;
				if (!dostream && data.size())
				{
					content = GetHTTPContentStart(&data[0], data.size());
					for (status = &data[0]; status < end-2; status++) if (*status == ' ') break;
					http_status = atoi(status+1);
				}
				size_t data_size = (content ? end - content : 0);

				if (sigReceivedString.HasConnections())
					sigReceivedString.call(http_status, (content ? ZL_String(content, data_size) : ZL_String::EmptyString));
				if (sigReceivedData.HasConnections())
					sigReceivedData.call(http_status, content, data_size);

				data.clear();
				Close();
				return false;
			}
		}
	}
};

struct ZL_WebSocket
{
	enum { OPCODE_CONTINUATION = 0, OPCODE_TEXT = 1, OPCODE_BINARY = 2, OPCODE_CONNECTIONCLOSE = 8, OPCODE_PING = 9, OPCODE_PONG = 10, _OPCODE_BITMASK = 0xF };

	static size_t SocketRecieve(ENetSocket sock, char*& buffer, size_t& bufferSize)
	{
		if (!buffer) buffer = (char*)malloc((bufferSize = 1024));
		ENetBuffer br;
		br.data = buffer;
		br.dataLength = bufferSize;
		size_t rec = 0;
		for (int r; (r = enet_socket_receive(sock, NULL, &br, 1)) > 0;)
		{
			rec += r;
			if ((size_t)r != br.dataLength) break;
			buffer = (char*)realloc(buffer, (bufferSize += 1024));
			br.data = buffer + rec;
			br.dataLength = 1024;
		}
		return rec;
	}

	static bool Read(char*& cursor, size_t& len, size_t& remain, size_t& opcode)
	{
		if (remain < 2 || ((cursor[0] & 0xF0) != 0x80)) return false; //need FIN 0x80 flag as fragmented frames are not supported
		size_t headerlen = 2 + ((cursor[1]&127)==127 ? 8 : ((cursor[1]&127)==126 ? 2 : 0));
		if (remain < headerlen) return false;
		len = (headerlen == 2 ? cursor[1]&127 : (headerlen == 4 ? ENET_NET_TO_HOST_16(*(unsigned short*)(cursor+2)) : ENET_NET_TO_HOST_32(*(unsigned int*)(cursor+6))));
		if (remain < headerlen+len) return false;
		bool hasMask = !!(cursor[1] & 0x80);
		opcode = (cursor[0] & _OPCODE_BITMASK);
		remain -= headerlen+len;
		cursor += headerlen;
		if (hasMask)
		{
			if (remain < 4) return false;
			char* mask = cursor; cursor += 4; remain -= 4;
			if (mask[0] || mask[1] || mask[2] || mask[3]) for (size_t i = 0; i != len; i++) cursor[i] ^= mask[i&3];
		}
		//ZL_LOG("WEBSOCK", "[RECV] OP: %d  [BINARY] LEN: %d [%02x %02x %02x %02x ...]", opcode, (int)len, ((char*)cursor)[0], ((char*)cursor)[1], ((char*)cursor)[2], ((char*)cursor)[3]);
		return true;
	}

	static void SendOp(ENetSocket sock, unsigned char opcode, const void* buf, size_t len, bool mask)
	{
		if (!sock) return;
		unsigned char header[2 + 8 + 4] = { (unsigned char)(0x80 | opcode), (unsigned char)(mask ? 0x80 : 0) }, headerlen = 2;
		if      (len > 0xFFFF) { headerlen += 8; header[1] |= 127; ((unsigned int*)(header+2))[0] = 0; ((unsigned int*)(header+2))[1] = ENET_HOST_TO_NET_32((unsigned int)len); }
		else if (len >    125) { headerlen += 2; header[1] |= 126; ((unsigned short*)header)[1] = ENET_HOST_TO_NET_16((unsigned short)len); }
		else header[1] |= len;
		if (mask) { *(unsigned int*)(header+headerlen) = 0; headerlen += 4; }
		ENetBuffer bs[2];
		bs[0].data = (void*)header; bs[0].dataLength = headerlen;
		bs[1].data = (void*)buf;    bs[1].dataLength = (unsigned int)len;
		//ZL_LOG("WEBSOCK", "[SEND] OP: %d  [BINARY] LEN: %d [%02x %02x %02x %02x ...]", opcode, (int)len, ((char*)buf)[0], ((char*)buf)[1], ((char*)buf)[2], ((char*)buf)[3]);
		enet_socket_send(sock, NULL, bs, 2);
	}
};

struct ZL_WebSocketServer_Impl : ZL_Connection_Impl, ZL_WebSocket
{
	ENetSocket listenSocket;
	ZL_Peer* peers;
	int numPeers;
	enet_uint8 packetData[ENET_PROTOCOL_MAXIMUM_MTU];
	char *buffer;
	size_t bufferSize;
	#define WEBSOCK_NO_HANDSHAKE_DATA ((void**)(this))

	ZL_Signal_v2<const ZL_Peer&, const ZL_String&> sigConnected;
	ZL_Signal_v2<const ZL_Peer&, unsigned short> sigDisconnected;
	ZL_Signal_v2<const ZL_Peer&, const ZL_String&> sigReceivedText;
	ZL_Signal_v3<const ZL_Peer&, const void*, size_t> sigReceivedBinary;

	ZL_WebSocketServer_Impl() : peers(NULL), numPeers(0), buffer(NULL) { }

	inline bool IsFullyConnected(ZL_Peer* peer) { return (peer->host && peer->data != WEBSOCK_NO_HANDSHAKE_DATA); }

	void Open(int server_port, unsigned short max_connections, const char *bind_host)
	{
		ZL_ASSERTMSG(OpenConnections, "ZL_Network::Init needs to be called before using network functions");

		listenSocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
		enet_socket_set_option(listenSocket, ENET_SOCKOPT_NONBLOCK, 1);
		enet_socket_set_option(listenSocket, ENET_SOCKOPT_NODELAY, 1);
		ENetAddress address;
		if (bind_host) enet_address_set_host(&address, bind_host);
		else address.host = ENET_HOST_ANY;
		address.port = server_port;
		if (!max_connections || listenSocket == ENET_SOCKET_NULL || enet_socket_bind(listenSocket, &address) < 0 || enet_socket_listen(listenSocket, max_connections) < 0)
		{
			enet_socket_destroy(listenSocket);
			return;
		}

		numPeers = max_connections;
		peers = (ZL_Peer*)malloc(sizeof(ZL_Peer) * numPeers);
		memset(peers, 0, sizeof(ZL_Peer) * numPeers);

		OpenConnectionsAdd();
	}

	void Close(short code = 1001) //1001: Going Away
	{
		if (!peers) return;
		code = ENET_HOST_TO_NET_16(code);
		for (ZL_Peer* peer = peers, *peerEnd = &peers[numPeers]; peer != peerEnd; peer++)
		{
			if (!peer->host) continue;
			SendOp((ENetSocket)(size_t)peer->handle, OPCODE_CONNECTIONCLOSE, &code, 2, false);
			enet_socket_destroy((ENetSocket)(size_t)peer->handle);
		}
		enet_socket_destroy(listenSocket);
		free(peers);
		peers = NULL;
		numPeers = 0;
		if (buffer) { free(buffer); buffer = NULL; }
		connectionWasClosed = true;
	}

protected:
	~ZL_WebSocketServer_Impl() { Close(); }
	virtual void ShutDown() { Close(1001); }

	virtual bool KeepAlive()
	{
		ENetAddress address;
		for (ENetSocket newClient; (newClient = enet_socket_accept(listenSocket, &address)) != ENET_SOCKET_NULL; newClient = ENET_SOCKET_NULL)
		{
			ZL_Peer* peer = peers, *peerEnd = &peers[numPeers];
			for (; peer != peerEnd; ++peer) if (!peer->host) break;
			if (peer == peerEnd) enet_socket_destroy(newClient);
			else
			{
				enet_socket_set_option(newClient, ENET_SOCKOPT_NONBLOCK, 1);
				enet_socket_set_option(newClient, ENET_SOCKOPT_NODELAY, 1);
				peer->host = ENET_NET_TO_HOST_32(address.host);
				peer->port = address.port;
				*const_cast<ZL_PeerHandle*>(&peer->handle) = (void*)(size_t)newClient;
				peer->data = WEBSOCK_NO_HANDSHAKE_DATA;
			}
		}
		for (ZL_Peer* peer = peers, *peerEnd = &peers[numPeers]; peer != peerEnd;)
		{
			unsigned int waitCondition = ENET_SOCKET_WAIT_RECEIVE;
			if (!peer->host || enet_socket_wait((ENetSocket)(size_t)peer->handle, &waitCondition, 0) || !(waitCondition & ENET_SOCKET_WAIT_RECEIVE))
			{
				peer++;
				continue;
			}

			size_t rec = SocketRecieve((ENetSocket)(size_t)peer->handle, buffer, bufferSize);
			if (rec < 2)
			{
				if (peer->data != WEBSOCK_NO_HANDSHAKE_DATA) sigDisconnected.call(*peer, 1006); //1006: Abnormal closure
				WEBSOCKETSERVERSHUTDOWNPEER:
				enet_socket_destroy((ENetSocket)(size_t)peer->handle);
				memset(peer, 0, sizeof(ZL_Peer));
				peer++;
				continue;
			}

			char* cursor = buffer;
			if (peer->data == WEBSOCK_NO_HANDSHAKE_DATA)
			{
				char* content = GetHTTPContentStart(buffer, rec);
				if (content) content[-1] = '\0'; //string terminator for strstr/strchr
				const char* getMethod       = (content             ? strstr(buffer, "GET ")                    : NULL);
				const char* getPathEnd      = (getMethod == buffer ? strchr(getMethod+4, ' ')                  : NULL);
				const char* webSocketKey    = (getPathEnd          ? strstr(getPathEnd, "Sec-WebSocket-Key: ") : NULL);
				const char* webSocketKeyEnd = (webSocketKey        ? strchr(webSocketKey, '\n')                : NULL);
				if (!webSocketKeyEnd) goto WEBSOCKETSERVERSHUTDOWNPEER;

				webSocketKey += sizeof("Sec-WebSocket-Key: ")-1;
				if (webSocketKeyEnd[-1] == '\r') webSocketKeyEnd--;

				ZL_String requestKey(webSocketKey, webSocketKeyEnd - webSocketKey), header;
				requestKey << "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
				unsigned char acceptKey[20];
				ZL_Checksum::SHA1(requestKey.c_str(), requestKey.size(), acceptKey);
				header << "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " << ZL_Base64::Encode(acceptKey, 20) << "\r\n\r\n";

				ENetBuffer bs;
				bs.data = (void*)header.c_str(); bs.dataLength = header.size();
				if (enet_socket_send((ENetSocket)(size_t)peer->handle, NULL, &bs, 1) <= 0) goto WEBSOCKETSERVERSHUTDOWNPEER;

				peer->data = NULL;
				sigConnected.call(*peer, ZL_String(getMethod+4, getPathEnd-getMethod-4));
				rec -= (content - buffer);
				cursor = content;
			}

			for (size_t len, opcode; Read(cursor, len, rec, opcode); cursor += len)
			{
				switch (opcode)
				{
					case OPCODE_TEXT:   if (sigReceivedText.HasConnections()) sigReceivedText.call(*peer, ZL_String(cursor, len)); break;
					case OPCODE_BINARY: if (sigReceivedBinary.HasConnections()) sigReceivedBinary.call(*peer, cursor, len); break;
					case OPCODE_PING:   SendOp((ENetSocket)(size_t)peer->handle, OPCODE_PONG, cursor, len, false); break;
					case OPCODE_CONNECTIONCLOSE: sigDisconnected.call(*peer, (len >= 2 ? ENET_NET_TO_HOST_16(*(unsigned short*)cursor) : 0)); goto WEBSOCKETSERVERSHUTDOWNPEER;
				}
			}
		}
		return true;
	}
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_WebSocketServer)

ZL_WebSocketServer::ZL_WebSocketServer(int server_port, unsigned short max_connections, const char *bind_host) : impl(NULL)
{
	Open(server_port, max_connections, bind_host);
}

void ZL_WebSocketServer::Open(int server_port, unsigned short max_connections, const char *bind_host)
{
	if (!impl) impl = new struct ZL_WebSocketServer_Impl();
	else impl->Close();
	impl->Open(server_port, max_connections, bind_host);
}

void ZL_WebSocketServer::Close(short code)
{
	if (impl) impl->Close(code);
}

void ZL_WebSocketServer::Send(ZL_PeerHandle peer, const void* data, size_t length, bool is_text)
{
	if (impl) ZL_WebSocket::SendOp((ENetSocket)(size_t)peer, (is_text ? ZL_WebSocket::OPCODE_TEXT : ZL_WebSocket::OPCODE_BINARY), data, length, false);
}

void ZL_WebSocketServer::Broadcast(const void* data, size_t length, bool is_text)
{
	if (impl) for (ZL_Peer *p = impl->peers, *pEnd = p + impl->numPeers; p != pEnd; p++)
		if (impl->IsFullyConnected(p)) ZL_WebSocket::SendOp((ENetSocket)(size_t)p->handle, (is_text ? ZL_WebSocket::OPCODE_TEXT : ZL_WebSocket::OPCODE_BINARY), data, length, false);
}

void ZL_WebSocketServer::Broadcast(const void* data, size_t length, ZL_PeerHandle except, bool is_text)
{
	if (impl) for (ZL_Peer *p = impl->peers, *pEnd = p + impl->numPeers; p != pEnd; p++)
		if (impl->IsFullyConnected(p) && p->handle != except) ZL_WebSocket::SendOp((ENetSocket)(size_t)p->handle, (is_text ? ZL_WebSocket::OPCODE_TEXT : ZL_WebSocket::OPCODE_BINARY), data, length, false);
}

void ZL_WebSocketServer::DisconnectPeer(ZL_PeerHandle peerhandle, short code)
{
	if (impl) for (ZL_Peer *p = impl->peers, *pEnd = p + impl->numPeers; p != pEnd; p++)
	{
		if (p->handle != peerhandle) continue;
		code = ENET_HOST_TO_NET_16(code);
		ZL_WebSocket::SendOp((ENetSocket)(size_t)p->handle, ZL_WebSocket::OPCODE_CONNECTIONCLOSE, &code, 2, false);
		enet_socket_destroy((ENetSocket)(size_t)p->handle);
		memset(p, 0, sizeof(ZL_Peer));
		return;
	}
}

void ZL_WebSocketServer::GetPeerHandles(std::vector<ZL_PeerHandle>& out_list)
{
	out_list.clear();
	if (impl) for (ZL_Peer *p = impl->peers, *pEnd = p + impl->numPeers; p != pEnd; p++)
		if (impl->IsFullyConnected(p)) out_list.push_back(p->handle);
}

void ZL_WebSocketServer::GetPeerDetails(std::vector<ZL_Peer>& out_list)
{
	out_list.clear();
	if (impl) for (ZL_Peer *p = impl->peers, *pEnd = p + impl->numPeers; p != pEnd; p++)
		if (impl->IsFullyConnected(p)) out_list.push_back(*p);
}

void ZL_WebSocketServer::SetPeerData(ZL_PeerHandle handle, void* data)
{
	if (impl) for (ZL_Peer *p = impl->peers, *pEnd = p + impl->numPeers; p != pEnd; p++)
		if (p->handle == handle) { p->data = data; return; }
}

void ZL_WebSocketServer::SetPeerData(const ZL_Peer& peer, void* data)
{
	((ZL_Peer*)&peer)->data = data;
}

bool ZL_WebSocketServer::IsOpened()
{
	return (impl && impl->peers);
}

size_t ZL_WebSocketServer::GetPeerCount()
{
	size_t res = 0;
	if (impl) for (const ZL_Peer *p = impl->peers, *pEnd = p + impl->numPeers; p != pEnd; p++)
		if (p->host) res++;
	return res;
}

ZL_Signal_v2<const ZL_Peer&, const ZL_String&>&    ZL_WebSocketServer::sigConnected()      { if (!impl) impl = new ZL_WebSocketServer_Impl(); return impl->sigConnected;      }
ZL_Signal_v2<const ZL_Peer&, unsigned short>&      ZL_WebSocketServer::sigDisconnected()   { if (!impl) impl = new ZL_WebSocketServer_Impl(); return impl->sigDisconnected;   }
ZL_Signal_v2<const ZL_Peer&, const ZL_String&>&    ZL_WebSocketServer::sigReceivedText()   { if (!impl) impl = new ZL_WebSocketServer_Impl(); return impl->sigReceivedText;   }
ZL_Signal_v3<const ZL_Peer&, const void*, size_t>& ZL_WebSocketServer::sigReceivedBinary() { if (!impl) impl = new ZL_WebSocketServer_Impl(); return impl->sigReceivedBinary; }

struct ZL_WebSocketClient_Impl : ZL_BasicTCPConnection_Impl, ZL_WebSocket
{
	ZL_String url;
	bool websocket_active;
	ZL_Signal_v0 sigConnected;
	ZL_Signal_v1<unsigned short> sigDisconnected;
	ZL_Signal_v1<const ZL_String&> sigReceivedText;
	ZL_Signal_v2<const void*, size_t> sigReceivedBinary;
	char *buffer;
	size_t bufferSize;

	ZL_WebSocketClient_Impl() : websocket_active(false), buffer(NULL) { }

	~ZL_WebSocketClient_Impl()
	{
		if (buffer) free(buffer);
	}

	void Connect(const char* url)
	{
		this->url = url;
		StartConnection();
	}

	void Disconnect(unsigned short code, const char* buf, size_t len)
	{
		code = ENET_HOST_TO_NET_16(code);
		if (socket && len)
		{
			unsigned short* sendbuf = (unsigned short*)malloc(2 + len); sendbuf[0] = code; memcpy(sendbuf+1, buf, len);
			SendOp(socket, OPCODE_CONNECTIONCLOSE, sendbuf, 2 + len, true);
			free(sendbuf);
		}
		else if (socket) SendOp(socket, OPCODE_CONNECTIONCLOSE, &code, 2, true);
		websocket_active = false;
		Close();
	}

	void Send(const void* buf, size_t len, bool is_text) { SendOp(socket, (is_text ? OPCODE_TEXT : OPCODE_BINARY), buf, len, true); }

protected:
	virtual bool GetHost(ZL_String& host, int& port)
	{
		return SplitUrl(url, 2, NULL, &host, &port); //only support "ws" protocol
	}

	virtual bool KeepAlive()
	{
		if (first_keep_alive)
		{
			if (thread_active) return true;
			if (GetRefCount() == 1) { Close(); delete this; return false; }
			DelRef();
			if (socket)
			{
				ZL_String header, path, host;
				if (!SplitUrl(url, 2, &path, &host)) return false; //only support "ws" protocol
				first_keep_alive = false;

				ENetBuffer bs;
				header << "GET " << path << " HTTP/1.1\r\nHost: " << host << "\r\nUser-Agent: ZillaLib/1.0\r\nSec-WebSocket-Version: 13\r\n"
					"Origin: null\r\nSec-WebSocket-Key: YPersQl07qSADJGSpuu5jw==\r\nConnection: keep-alive, Upgrade\r\nPragma: no-cache\r\nCache-Control: no-cache\r\nUpgrade: websocket\r\n\r\n";
				bs.data = (void*)header.c_str(); bs.dataLength = header.size();
				if (enet_socket_send(socket, NULL, &bs, 1) <= 0) return false;
			}
		}
		if (!socket)
		{
			WEBSOCKETCLIENTABORTED:
			sigDisconnected.call(1006); //1006: Abnormal closure
			WEBSOCKETCLIENTCLOSED:
			websocket_active = false;
			Close();
			return false;
		}
		for (;;)
		{
			unsigned int waitCondition = ENET_SOCKET_WAIT_RECEIVE;
			if (enet_socket_wait(socket, &waitCondition, 0) || !(waitCondition & ENET_SOCKET_WAIT_RECEIVE)) return true;

			size_t rec = SocketRecieve(socket, buffer, bufferSize);
			if (rec < 2) goto WEBSOCKETCLIENTABORTED;

			char* cursor = buffer;
			if (!websocket_active)
			{
				char* content = GetHTTPContentStart(buffer, rec);
				if (content) content[-1] = '\0'; //string terminator for strstr/strchr
				const char* protocol    = (content            ? strstr(buffer, "HTTP")       : NULL);
				const char* protocolEnd = (protocol == buffer ? strchr(protocol+4, ' ')      : NULL);
				const char* status101   = (protocolEnd        ? strstr(protocolEnd, " 101 ") : NULL);
				if (!status101 || status101 != protocolEnd) goto WEBSOCKETCLIENTABORTED;
				websocket_active = true;
				sigConnected.call();
				rec -= content - buffer;
				cursor = content;
			}

			for (size_t len, opcode; websocket_active && Read(cursor, len, rec, opcode); cursor += len)
			{
				switch (opcode)
				{
					case OPCODE_TEXT:   if (sigReceivedText.HasConnections()) sigReceivedText.call(ZL_String(cursor, len)); break;
					case OPCODE_BINARY: if (sigReceivedBinary.HasConnections()) sigReceivedBinary.call(cursor, len); break;
					case OPCODE_PING:   SendOp(socket, OPCODE_PONG, cursor, len, true); break;
					case OPCODE_CONNECTIONCLOSE: sigDisconnected.call(len >= 2 ? ENET_NET_TO_HOST_16(*(unsigned short*)cursor) : 0); goto WEBSOCKETCLIENTCLOSED;
				}
			}
		}
	}
};

#else //ZL_USE_ENET

bool ZL_Network::Init() { return true; }
void ZL_Network::DeInit() { }

ZL_WEBSOCKETCLIENT_IMPL_INTERFACE
ZL_HTTPCONNECTION_IMPL_INTERFACE

#endif //ZL_USE_ENET

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_WebSocketClient)
ZL_WebSocketClient::ZL_WebSocketClient(const char *url) : impl(NULL) { Connect(url); }
void ZL_WebSocketClient::Connect(const char *url) { if (!url) return; if (!impl) impl = new ZL_WebSocketClient_Impl(); impl->Connect(url); }
void ZL_WebSocketClient::Disconnect(unsigned short code, const char *reason, size_t reason_length) const { if (impl) impl->Disconnect(code, reason, reason_length); }
bool ZL_WebSocketClient::IsConnected() const { return (impl && impl->websocket_active); }
void ZL_WebSocketClient::Send(const void* data, size_t length, bool is_text) { if (IsConnected()) impl->Send(data, length, is_text); }
ZL_Signal_v0&                      ZL_WebSocketClient::sigConnected()      { if (!impl) impl = new ZL_WebSocketClient_Impl(); return impl->sigConnected;      }
ZL_Signal_v1<unsigned short>&      ZL_WebSocketClient::sigDisconnected()   { if (!impl) impl = new ZL_WebSocketClient_Impl(); return impl->sigDisconnected;   }
ZL_Signal_v1<const ZL_String&>&    ZL_WebSocketClient::sigReceivedText()   { if (!impl) impl = new ZL_WebSocketClient_Impl(); return impl->sigReceivedText;   }
ZL_Signal_v2<const void*, size_t>& ZL_WebSocketClient::sigReceivedBinary() { if (!impl) impl = new ZL_WebSocketClient_Impl(); return impl->sigReceivedBinary; }

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_HttpConnection)
ZL_HttpConnection::ZL_HttpConnection(const char *url) : impl(NULL) { Connect(url); }
ZL_HttpConnection& ZL_HttpConnection::ClearPostData() { if (impl) impl->post_data.clear(); return *this; }
ZL_HttpConnection& ZL_HttpConnection::SetPostData(const char *data) { return SetPostData(data, strlen(data)); }
ZL_HttpConnection& ZL_HttpConnection::SetPostData(const void* data, size_t length) { if (!impl) impl = new ZL_HttpConnection_Impl(); impl->post_data.resize(length); memcpy(&impl->post_data[0], data, length); return *this; }
ZL_HttpConnection& ZL_HttpConnection::SetTimeout(unsigned int timeout_msec)        { if (!impl) impl = new ZL_HttpConnection_Impl(); impl->timeout_msec = timeout_msec; return *this; }
ZL_HttpConnection& ZL_HttpConnection::SetDoStreamData(bool DoStreamData)           { if (!impl) impl = new ZL_HttpConnection_Impl(); impl->dostream = DoStreamData; return *this; }
void ZL_HttpConnection::Connect(const char *url) { if (!url) return; if (!impl) impl = new ZL_HttpConnection_Impl(); impl->Connect(url); }
ZL_Signal_v2<int, const ZL_String&>&    ZL_HttpConnection::sigReceivedString() { if (!impl) impl = new ZL_HttpConnection_Impl(); return impl->sigReceivedString; }
ZL_Signal_v3<int, const char*, size_t>& ZL_HttpConnection::sigReceivedData()   { if (!impl) impl = new ZL_HttpConnection_Impl(); return impl->sigReceivedData;   }

#endif