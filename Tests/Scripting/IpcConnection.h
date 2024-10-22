#pragma once
#include "Containers/AxArray.h"
#include "Core/Types.h"

namespace apex {
namespace ipc {

	enum class IpcSocketType : uint8
	{
		eStream,
		eDatagram,
	};

	enum class IpcSocketProtocol : uint8
	{
		eTcp,
		eUdp,
	};

	struct IpcSocketOptions;

	class IpcSocket
	{
	private:
		static constexpr uint64 kInvalidHandle = ~0ull;

	public:
		struct InvalidSocket {};

		IpcSocket(InvalidSocket = {}) : m_handle(kInvalidHandle) {}
		~IpcSocket();

		IpcSocket(IpcSocket const&) = delete;
		IpcSocket& operator=(IpcSocket const&) = delete;

		IpcSocket(IpcSocket&& other) noexcept
		{
			m_handle = other.m_handle;
			other.m_handle = kInvalidHandle;
		}

		IpcSocket& operator=(IpcSocket&& other) noexcept
		{
			m_handle = other.m_handle;
			other.m_handle = kInvalidHandle;
			return *this;
		}

		bool IsValid() const { return m_handle != kInvalidHandle; }

		bool Open(IpcSocketType type, IpcSocketProtocol protocol, uint16 port, bool nonBlocking);
		void Close();

		bool Listen(uint16 port);
		bool Accept(IpcSocket& client_socket) const;

		uint32 Recv(AxArrayRef<char> buffer);
		uint32 Send();

		uint64 GetRawHandle() const { return m_handle; }
		void* GetRawHandleAsPointer() const { return m_handlePtr; }

	private:
		explicit IpcSocket(uint64 handle) : m_handle(handle) {}
		explicit IpcSocket(void* handle) : m_handlePtr(handle) {}

		friend IpcSocket CreateSocket(IpcSocketOptions);

	private:
		union
		{
			void*		m_handlePtr;
			uint64		m_handle;
		};
	};

	struct IpcSocketOptions
	{
		IpcSocketType socketType;
		IpcSocketProtocol socketProtocol;
		bool nonBlocking { true };
	};

	bool Initialize();

	IpcSocket CreateSocket(IpcSocketOptions options);

	void TestWinsock();
}
}


