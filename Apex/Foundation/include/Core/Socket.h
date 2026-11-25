#pragma once
#include "Types.h"
#include "Containers/AxArray.h"
#include "String/AxString.h"

namespace apex {
namespace ipc {

	class Socket
	{
	public:
		static void Init();
		static void Shutdown();

		constexpr static u64 kInvalidSocket = static_cast<u32>(-1);

		Socket() : m_impl(kInvalidSocket) {}
		~Socket();

		Socket(const Socket&);
		Socket& operator=(const Socket&);

		Socket(Socket&&) noexcept;
		Socket& operator=(Socket&&) noexcept;

		[[nodiscard]] bool Bind() const;
		[[maybe_unused]] bool Listen(int backlog = -1) const;
		Socket Accept() const;
		Socket AcceptPoll(int timeoutMillis) const;
		[[maybe_unused]]bool Close() const;

		int Send(const char* buf, int buflen) const;
		int Recv(char* buf, int buflen) const;
		
		template <typename Type>
		int SendBytes(AxSpan<Type> buffer)
		{
			return Send(static_cast<char*>(buffer.data()), buffer.size() * sizeof(Type));
		}

		template <typename Type>
		int RecvBytes(AxSpan<Type> buffer)
		{
			return Recv(static_cast<char*>(buffer.data()), buffer.size() * sizeof(Type));
		}

		AxString GetAddr() const;
		bool GetAddr(char* buf, int buflen) const;
		u16 GetPort() const;

		bool IsValid() const { return m_impl != kInvalidSocket; }
		
		explicit operator bool() const { return IsValid(); }

	private:
		Socket(u32 impl);

		u32 m_impl;

		friend Socket CreateSocket(u16);
	};

	Socket CreateSocket(u16 port);
	

}
}
