//
// Created by jayphen on 22/9/2023.
//

#include "jlog/os.h"

bool jlog::detail::os::Write(StreamDescriptor stream, const std::span<const uint8_t> data) {
	if (!stream) {
		return false;
	}
#ifdef JLOG_UNIX_LIKE
	const bool result = fwrite(data.data(), sizeof(uint8_t), data.size(), (FILE*)stream) == data.size();
	fflush((FILE*)stream);
	return result;
#elifdef JLOG_WINDOWS
	DWORD written;
	return WriteFile((HANDLE)stream, data.data(), data.size(), &written, nullptr);
#endif
}

jlog::detail::os::StreamDescriptor jlog::detail::os::ConsoleStream::GetDescriptor() const {
	return this->m_Descriptor;
}

jlog::detail::os::ConsoleStream::ConsoleStream()
	:
#ifdef JLOG_UNIX_LIKE
	  m_Descriptor(stdout)
#elifdef JLOG_WINDOWS
	  m_Descriptor(GetStdHandle(STD_OUTPUT_HANDLE))
#endif
{
}

jlog::detail::os::FileStream::FileStream(std::filesystem::path&& file_path) {
#ifdef JLOG_UNIX_LIKE
	this->m_Descriptor = fopen(file_path.string().c_str(), "a");
#elifdef JLOG_WINDOWS
	this->m_Descriptor = CreateFileW(file_path.wstring().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
									FILE_ATTRIBUTE_NORMAL, NULL);
#endif
}

jlog::detail::os::FileStream::~FileStream() {
#ifdef JLOG_UNIX_LIKE
	if (this->m_Descriptor) {
		fclose((FILE*)this->m_Descriptor);
	}
#elifdef JLOG_WINDOWS
	if (this->m_Descriptor) {
		CloseHandle(this->m_Descriptor);
	}
#endif
}

jlog::detail::os::StreamDescriptor jlog::detail::os::FileStream::GetDescriptor() const {
	return this->m_Descriptor;
}
jlog::detail::os::FileStream::FileStream(jlog::detail::os::FileStream&& other) noexcept {
	this->m_Descriptor = other.m_Descriptor;
	other.m_Descriptor = nullptr;
}
jlog::detail::os::StreamDescriptor jlog::detail::os::NullStream::GetDescriptor() {
	return nullptr;
}
jlog::detail::os::NullStream::NullStream() = default;
