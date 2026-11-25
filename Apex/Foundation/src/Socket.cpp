#include "Core/Socket.h"

#include "Memory/AxPool.h"

#ifdef APEX_PLATFORM_WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif // APEX_PLATFORM_WIN32

namespace apex::ipc {

#ifdef APEX_CONFIG_DEBUG
#define AssertSocketInitialized() axAssertFmt(detail::s_initialized, "Socket lib was not initialzed! Did you call ipc::Socket::Init() ?");
#else
#define AssertSocketInitialized()
#endif // APEX_CONFIG_DEBUG

	namespace detail
	{
		struct SocketImpl
		{
#ifdef APEX_PLATFORM_WIN32
			SOCKET sock;
			sockaddr_in addr; // IPv4 socket address
			atomic_u32 refCount;
#endif
		};

#ifdef APEX_CONFIG_DEBUG
		static bool s_initialized = false;
#endif
		static WSADATA s_wsaData;
		static mem::AxPool<SocketImpl, std::mutex> s_socketPool;
	}

	static const char* FormatErrorMessage(int errorCode)
	{
#ifdef APEX_PLATFORM_WIN32
		static char MSGBUF[256];
		ZeroMemory(MSGBUF, sizeof(MSGBUF));
		DWORD result = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			MSGBUF,
			sizeof(MSGBUF),
			nullptr);
		return MSGBUF;
#endif
	}

	void Socket::Init()
	{
		int res = WSAStartup(MAKEWORD(2,2), &detail::s_wsaData);
		axCriticalAssertFmt(res == 0, "WinSock: WSAStartup failed: {}. It is not safe to continue.", res);
		detail::s_socketPool.Init(64);
		detail::s_initialized = true;
	}

	void Socket::Shutdown()
	{
		axAssertFmt(detail::s_socketPool.GetActiveCount() == 0, "Pool contains active elements!");
		detail::s_socketPool.Shutdown();
		WSACleanup();
	}

	Socket::Socket(u32 impl)
		: m_impl(impl)
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(impl);
		++socket->refCount;
	}

	Socket::~Socket()
	{
		if (m_impl != kInvalidSocket)
		{
			detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
			if (--socket->refCount == 0)
			{
				closesocket(socket->sock);
				detail::s_socketPool.Free(socket);
			}
		}
	}

	Socket::Socket(const Socket& other)
		: m_impl(other.m_impl)
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		++socket->refCount;
	}

	Socket& Socket::operator=(const Socket& other)
	{
		if (this != &other)
		{
			m_impl = other.m_impl;
			detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
			++socket->refCount;
		}
		return *this;
	}

	Socket::Socket(Socket&& other) noexcept
		: m_impl(other.m_impl)
	{
		other.m_impl = kInvalidSocket;
	}

	Socket& Socket::operator=(Socket&& other) noexcept
	{
		m_impl = other.m_impl;
		other.m_impl = kInvalidSocket;
		return *this;
	}

	bool Socket::Bind() const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		int res = bind(socket->sock, (const sockaddr*)&socket->addr, sizeof(socket->addr));
		if (axVerifyFmt(res != SOCKET_ERROR, "WinSock: Error in bind(): {}", FormatErrorMessage(WSAGetLastError())))
		{
			return true;
		}
		return false;
	}

	bool Socket::Listen(int backlog) const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		int res = listen(socket->sock, backlog);
		if (axVerifyFmt(res != SOCKET_ERROR, "WinSock: Error in listen(): {}", FormatErrorMessage(WSAGetLastError())))
		{
			return true;
		}
		return false;
	}

	Socket Socket::Accept() const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		sockaddr_in addr;
		int addrlen = sizeof(addr);
		SOCKET handle = accept(socket->sock, (sockaddr*)&addr, &addrlen);
		if (!axVerifyFmt(handle != INVALID_SOCKET, "WinSock: Error in accept(): {}", FormatErrorMessage(WSAGetLastError())))
		{
			return {};
		}
		detail::SocketImpl* newSocket = detail::s_socketPool.New();
		newSocket->sock = handle;
		apex::memcpy_s<sockaddr_in>(&newSocket->addr, 1, &addr, 1);
		newSocket->refCount = 0;

		return { detail::s_socketPool.GetIndex(newSocket) };
	}

	Socket Socket::AcceptPoll(int timeoutMillis) const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		WSAPOLLFD pollfd {.fd = socket->sock, .events = POLLRDNORM };
		int pollResult = WSAPoll(&pollfd, 1, timeoutMillis);
		if (axVerifyFmt(pollResult != SOCKET_ERROR, "WinSock: Error in WSAPoll: {}", FormatErrorMessage(WSAGetLastError()))
			&& pollResult != 0 && (pollfd.revents & POLLRDNORM) == POLLRDNORM)
		{
			return Accept();
		}
		return {};
	}

	bool Socket::Close() const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		int res = shutdown(socket->sock, SD_SEND);
		if (!axVerifyFmt(res != SOCKET_ERROR, "WinSock: Error in shutdown(): {}", FormatErrorMessage(WSAGetLastError())))
		{
			return false;
		}
		return true;
	}

	int Socket::Send(const char* buf, int buflen) const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		int res = send(socket->sock, buf, buflen, 0);
		axAssertFmt(res != SOCKET_ERROR, "WinSock: Error in send(): {}", FormatErrorMessage(WSAGetLastError()));
		return res;
	}

	int Socket::Recv(char* buf, int buflen) const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		int res = recv(socket->sock, buf, buflen, 0);
		axAssertFmt(res != SOCKET_ERROR, "WinSock: Error in recv(): {}", FormatErrorMessage(WSAGetLastError()));
		return res;
	}

	AxString Socket::GetAddr() const
	{
		AxString addr(INET_ADDRSTRLEN);
		GetAddr(addr.data(), addr.capacity());
		return addr;
	}

	bool Socket::GetAddr(char* buf, int buflen) const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		if (axVerify(inet_ntop(AF_INET, &socket->addr.sin_addr, buf, buflen) != nullptr))
		{
			return true;
		}
		return false;
	}

	u16 Socket::GetPort() const
	{
		detail::SocketImpl* socket = detail::s_socketPool.GetPtr(m_impl);
		return ntohs(socket->addr.sin_port);
	}

	Socket CreateSocket(u16 port)
	{
		AssertSocketInitialized()
#ifdef APEX_PLATFORM_WIN32
		int res;
		addrinfo *result = nullptr, hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		char szPort[6];
		(void)sprintf_s(szPort, "%u", (u32)port);
		res = getaddrinfo(NULL, szPort, &hints, &result);
		if (!axVerifyFmt(res == 0, "WinSock: Error in getaddrinfo(): {}", res))
		{
			return {};
		}

		SOCKET handle = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (!axVerifyFmt(handle != INVALID_SOCKET, "WinSock: Error in getaddrinfo(): {}", res))
		{
			freeaddrinfo(result);
			return {};
		}

		detail::SocketImpl* sock = detail::s_socketPool.New();
		sock->sock = handle;
		axAssert(result->ai_addrlen == sizeof(sock->addr));
		apex::memcpy_s<sockaddr_in>(&sock->addr, 1, result->ai_addr, 1);
		sock->refCount = 0;

		freeaddrinfo(result);
		return { detail::s_socketPool.GetIndex(sock) };
#else
#error Not implemented
#endif
	}
}
