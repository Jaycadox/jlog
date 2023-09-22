//
// Created by jayphen on 22/9/2023.
//

#ifndef JLOG_OS_H
#define JLOG_OS_H

#include <cstdio>
#include <filesystem>
#include <span>
#include <variant>

#if defined(__APPLE__) || defined(__POSIX__) || defined(__UNIX__)
#define JLOG_UNIX_LIKE
#include <unistd.h>
using StreamDescriptor = FILE*;
#elif defined(_WIN32) || defined(_WIN64)
#define JLOG_WINDOWS
#include <windows.h>
using StreamDescriptor = HANDLE;
#endif

#if !defined(JLOG_WINDOWS) && !defined(JLOG_UNIX_LIKE)
static_assert(false, "Unable to determine target platform");
#endif

namespace jlog::detail::os {

using StreamDescriptor = void*;

class ConsoleStream {
   private:
	StreamDescriptor m_Descriptor;

   public:
	ConsoleStream();
	ConsoleStream(ConsoleStream&&) = default;
	ConsoleStream(const ConsoleStream&) = delete;
	[[nodiscard]] StreamDescriptor GetDescriptor() const;
};

class FileStream {
   private:
	StreamDescriptor m_Descriptor;

   public:
	explicit FileStream(std::filesystem::path&& file_path);
	FileStream(FileStream&&) noexcept;
	FileStream(const FileStream&) = delete;
	FileStream& operator=(const FileStream&) = delete;
	~FileStream();
	[[nodiscard]] StreamDescriptor GetDescriptor() const;
};

class NullStream {
   public:
	NullStream();
	[[nodiscard]] static StreamDescriptor GetDescriptor();
};

using Stream = std::variant<FileStream, ConsoleStream, NullStream>;

bool Write(StreamDescriptor stream, std::span<const uint8_t> data);

}  // namespace jlog::detail::os

#endif	// JLOG_OS_H
