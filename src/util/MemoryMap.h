#pragma once

#include <filesystem>
#include <utility>

#include "Log.h"
#include "Types.h"

#ifdef _WIN32
#pragma comment(lib, "onecore.lib")

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#ifdef _WIN32
inline void LogError() {
	
    DWORD error_id = ::GetLastError();
    if (!error_id) return;

	auto LocalFreeDeleter = [](HLOCAL handle) { ::LocalFree(handle); };
    std::unique_ptr<CHAR, decltype(LocalFreeDeleter)> message_buffer;
    size_t size{0};
    {
        LPSTR message_buffer_raw{};
        size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, error_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                reinterpret_cast<LPSTR>(&message_buffer_raw), 0, NULL);
        message_buffer.reset(message_buffer_raw);
    }
    LB_ERROR(Memory, "{:#010X}: {}", static_cast<u32>(error_id), std::string_view{message_buffer.get(), size});
}
#endif

class ReservedSpace {
private:
	void* data;
	size_t size;

public:
	ReservedSpace(size_t s) : size(s) {
#ifdef _WIN32
		data = VirtualAlloc2(GetCurrentProcess(), nullptr, size, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, nullptr, 0);
		if (!data) LogError(); 
#endif
	}

	~ReservedSpace() {
#ifdef _WIN32
		VirtualFree(data, 0, MEM_RELEASE);
#endif
	}
	
	void split(size_t offset, size_t s) {
#ifdef _WIN32
		VirtualFree(static_cast<u8*>(data) + offset, s, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER);
#endif
	}
	
	void unsplit(size_t offset, size_t s) {
#ifdef _WIN32
		VirtualFree(static_cast<u8*>(data) + offset, s, MEM_RELEASE | MEM_COALESCE_PLACEHOLDERS);
#endif
	}

	template<typename T = u8>
	T* get() {
		return static_cast<T*>(data);
	}
};

class Access {
private:
	u8 flags;

public:
	enum Flags : decltype(flags) {
		Read = 1 << 0,
		Write = 1 << 1,
		RW = Read | Write
	};

	explicit Access(Flags f) : flags(f) {}
    Access(Access&&) = default;
    Access& operator=(Access&&) = default;

	bool operator&(Flags f) const { return flags & f; }

#ifdef _WIN32
	DWORD toGeneric() const {
		DWORD res = 0;
		if (flags & Read) res |= GENERIC_READ;
		if (flags & Write) res |= GENERIC_WRITE;
		return res;
	}

	DWORD toFileMap() const {
		DWORD res = 0;
		if (flags & Read) res |= FILE_MAP_READ;
		if (flags & Write) res |= FILE_MAP_WRITE;
		return res;
	}

	DWORD toPage() const {
		if (flags & Write) return PAGE_READWRITE;
		if (flags & Read) return PAGE_READONLY;
		return PAGE_NOACCESS;
	}
#endif
};

#ifdef _WIN32

inline auto closehandle = [](HANDLE handle) { CloseHandle(handle); };
inline auto unmap = [](void* d) { UnmapViewOfFile(d); };
inline auto preserve = [](void* d) { UnmapViewOfFile2(GetCurrentProcess(), d, MEM_PRESERVE_PLACEHOLDER); };

//extern "C" __declspec(dllimport) NTSTATUS WINAPI RtlGetLastNtStatus();

class MemoryMap {
private:
	using WinHandle = std::unique_ptr<void, decltype(closehandle)>;
	WinHandle file = NULL;
	WinHandle mapping = NULL;

	Access access {Access::RW};

public:
	template <typename Unmapper>
	class GenericSection {
		std::unique_ptr<void, Unmapper> ptr{};

	public:
		GenericSection() = default;
		GenericSection(GenericSection&&) = default;
		GenericSection(void* ptr) : ptr(ptr) {}
		GenericSection& operator=(GenericSection&&) = default;

		template <typename T = u8>
		T* get() {
			return static_cast<T*>(ptr.get());
		}

		u8& operator[](size_t offset) {
			return get()[offset];
		}
	};

	using Section = GenericSection<decltype(unmap)>;
	using ReservedSection = GenericSection<decltype(preserve)>;

	MemoryMap() = default;
	MemoryMap(MemoryMap&&) = default;
    MemoryMap& operator=(MemoryMap&&) = default;

	MemoryMap(const std::filesystem::path& p, Access::Flags a, size_t size)
		: access(a) {
		create(size, open(p));
	}

	MemoryMap(size_t size) : access(Access::RW) {
		create(size);
	}

	~MemoryMap() {
		close();
	}

	void close() {
		mapping = {};
		file = {};
	}

	// default map entire section
	Section map(size_t offset = 0, size_t size = 0) {
		void* ptr = MapViewOfFile2(
			mapping.get(), 
			GetCurrentProcess(), 
			offset, nullptr, 
			size, 0, 
			access.toPage()
		);

		if (!ptr) {
			//LB_ERROR(Memory, "{:#010X}", static_cast<u32>(RtlGetLastNtStatus()));
        	LogError();
		}

		return ptr;
	}

	ReservedSection map(void* base, size_t offset = 0, size_t size = 0) {
		void* ptr = MapViewOfFile3(
			mapping.get(), 
			GetCurrentProcess(), 
			base, offset, 
			size, MEM_REPLACE_PLACEHOLDER, 
			access.toPage(), 
			nullptr, 0
		);

		
		if (!ptr) {
			//LB_ERROR(Memory, "{:#010X}", static_cast<u32>(RtlGetLastNtStatus()));
       		LogError();
		}

		return ptr;
	}

private:
	void create(size_t size, HANDLE handle = INVALID_HANDLE_VALUE) {
		mapping.reset(CreateFileMapping2(
			handle, 
			nullptr,
			access.toFileMap(),
			access.toPage(), 
			SEC_COMMIT, 
			size,
			nullptr, 
			nullptr, 0
		));

		if (!mapping) LogError();
	}

	HANDLE open(const std::filesystem::path& p) {
		DWORD dwAccess = access.toGeneric();
		DWORD dwShare = (access & Access::Write) ? 0 : FILE_SHARE_READ;

		file.reset(CreateFileW(p.c_str(), dwAccess, dwShare, nullptr, OPEN_EXISTING,
						   FILE_FLAG_RANDOM_ACCESS, nullptr));

		if (file.get() == INVALID_HANDLE_VALUE) LogError();

		return file.get();
	}
};

#else

#error "No MemoryMap implementation"

#endif
