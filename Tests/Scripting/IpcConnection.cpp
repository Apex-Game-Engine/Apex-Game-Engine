#include "IpcConnection.h"
#include "Core/Asserts.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Containers/AxArray.h"


#pragma comment(lib, "Ws2_32.lib")

namespace apex::ipc {

	bool Initialize()
	{
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (0 != result)
		{
			axAssertMsg(0 == result, "WSAStartup failed : {}", result);
			return false;
		}
		axDebug("IPC Initialized for Windows using WinSock2");
		return true;
	}

	enum ErrorValues : int64
	{
		IpcErrInvalidSocket = 0,
		IpcErrSocketError = 0,
		IpcErrInvalidUsage = 0,
	};

	IpcSocket CreateSocket(IpcSocketOptions options)
	{
		int sockType = -1, proto = -1;

		switch (options.socketType)
		{
		case IpcSocketType::eStream:	sockType = SOCK_STREAM; 	break;
		case IpcSocketType::eDatagram:	sockType = SOCK_DGRAM;		break;
		}

		switch (options.socketProtocol)
		{
		case IpcSocketProtocol::eTcp:	proto = IPPROTO_TCP;	break;
		case IpcSocketProtocol::eUdp:	proto = IPPROTO_UDP;	break;
		}

		SOCKET _socket = socket(AF_INET, sockType, proto);
		if (INVALID_SOCKET == _socket)
		{
			axAssertMsg(IpcErrInvalidSocket, "Error creating socket : {}", WSAGetLastError());
			return IpcSocket::InvalidSocket{};
		}

		if (options.nonBlocking)
		{
			u_long mode = 1;
			if (SOCKET_ERROR == ioctlsocket(_socket, FIONBIO, &mode))
			{
				axAssertMsg(IpcErrSocketError, "Failed to set socket to non-blocking I/O mode : {}", WSAGetLastError());
				return IpcSocket::InvalidSocket{};
			}
		}

		return IpcSocket(_socket);
	}

	void TestWinsock()
	{
		// Startup
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (0 != result)
		{
			axAssertMsg(0 == result, "WSAStartup failed : {}", result);
			return;
		}
		axDebug("IPC Initialized for Windows using WinSock2");

		// Create socket
		SOCKET _socket = INVALID_SOCKET;
		IpcSocket sock = CreateSocket({IpcSocketType::eStream, IpcSocketProtocol::eTcp, true});
		_socket = sock.GetRawHandle();
		if (INVALID_SOCKET == _socket)
		{
			closesocket(_socket);
			WSACleanup();
			return;
		}

		// Bind socket
		sockaddr_in addr {};
		// memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(21000);

		result = bind(_socket, (const sockaddr*)&addr, sizeof(addr));
		if (SOCKET_ERROR == result)
		{
			axAssertMsg(SOCKET_ERROR != result, "Failed to bind socket : {}", WSAGetLastError());
			closesocket(_socket);
			WSACleanup();
			return;
		}

		axInfo("Socket created and bound successfully!");
		closesocket(_socket);
		WSACleanup();
	}

	IpcSocket::~IpcSocket()
	{
		if (IsValid())
		{
			Close();
		}
	}

	bool IpcSocket::Open(IpcSocketType type, IpcSocketProtocol protocol, uint16 port, bool nonBlocking)
	{
		if (IsValid())
		{
			axAssertMsg(IpcErrInvalidUsage, "Attempting to open a new socket while one already exists!");
			return false;
		}

		m_handle = CreateSocket({ type, protocol, nonBlocking }).m_handle;

		if (!IsValid())
		{
			return false;
		}

		return Listen(port);
	}

	void IpcSocket::Close()
	{
		if (SOCKET_ERROR == closesocket(m_handle))
		{
			axAssertMsg(IpcErrSocketError, "Failed to close socket : {}", WSAGetLastError());
		}
		m_handle = kInvalidHandle;
	}

	bool IpcSocket::Listen(uint16 port)
	{
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		if (SOCKET_ERROR == bind(m_handle, (const sockaddr*)&addr, sizeof(addr)))
		{
			char s[512];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
			               nullptr, WSAGetLastError(),
			               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			               (LPSTR)&s, sizeof(s), nullptr);
			axAssertMsg(IpcErrSocketError, "Failed to bind socket : ({}) {}", WSAGetLastError(), s);
			Close();
			return false;
		}

		if (SOCKET_ERROR == listen(m_handle, SOMAXCONN))
		{
			axAssertMsg(IpcErrSocketError, "Failed to listen to socket : {}", WSAGetLastError());
			Close();
			return false;
		}

		return true;
	}

	bool IpcSocket::Accept(IpcSocket& client_socket) const
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(m_handle, &readfds);

		timeval timeout = {0, 100 };

		int activity = select(0, &readfds, nullptr, nullptr, &timeout);

		if (activity > 0 && FD_ISSET(m_handle, &readfds))
		{
			SOCKET clientSocket = accept(m_handle, nullptr, nullptr);
			if (INVALID_SOCKET == clientSocket)
			{
				axAssertMsg(IpcErrInvalidSocket, "Error connecting to client socket : {}", WSAGetLastError());
				client_socket = InvalidSocket{};
				return false;
			}
			client_socket.m_handle = clientSocket;
			return true;
		}

		return false;
	}

	uint32 IpcSocket::Recv(AxArrayRef<char> buffer)
	{
		int result = recv(m_handle, buffer.data, buffer.count, 0);
		if (result > 0)
		{
			buffer.data[result] = 0;
			return result;
		}
		if (0 == result)
		{
			axInfoFmt("Client disconnected");
		}
		else
		{
			int errorCode = WSAGetLastError();
			if (WSAEWOULDBLOCK == errorCode)
			{
				return 0;
			}
			axAssertMsg(IpcErrSocketError, "Failed to recv from socket : {}", WSAGetLastError());
		}
		Close();
		return 0;
	}

	uint32 IpcSocket::Send()
	{
		return 0;
	}
}
