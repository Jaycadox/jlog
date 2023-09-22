//
// Created by jayphen on 22/9/2023.
//

#ifndef JLOG_TAP_H
#define JLOG_TAP_H

#include <fmt/format.h>
#include <fmt/xchar.h>
#include <jlog/os.h>
#include <atomic>
#include <queue>
#include <thread>
#include <functional>
#include "jlog/string_help.h"

namespace jlog::detail::tap {

template <typename T>
struct DeferedString {
	using Type = T;
	std::function<std::basic_string<T>(void)> callback;
};

using GeneralDeferedString = std::variant<DeferedString<char>, DeferedString<wchar_t>>;

template <size_t N>
struct StringLiteral {
	constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

	char value[N];
};

template <StringLiteral Str, typename T, typename... Args>
auto DeferFormat(Args&&... args) {
	return DeferedString<T>{[args...]() { return fmt::format(Str.value, args...); }};
}

class SyncTap {
	os::StreamDescriptor m_Descriptor;
	os::Stream m_Stream;

   public:
	explicit SyncTap(jlog::detail::os::Stream stream);

	template <typename T>
	void Process(DeferedString<T>&& str) {
		const auto formatted_string = str.callback();
		os::Write(this->m_Descriptor, string::StringViewToByteSpan<T>(formatted_string));
	}
};

class AsyncTap {
	os::StreamDescriptor m_Descriptor;
	std::atomic_bool m_ThreadRunning;
	std::thread m_LogThread;
	std::queue<GeneralDeferedString> m_MessageQueue;
	std::mutex m_MessageQueueMutex;
	os::Stream m_Stream;

   public:
	explicit AsyncTap(jlog::detail::os::Stream&& stream);
	~AsyncTap();

	template <typename T>
	void Process(DeferedString<T>&& str) {
		std::lock_guard lg(m_MessageQueueMutex);
		m_MessageQueue.emplace(str);
	}

   private:
	void FlushQueue();
};

}  // namespace jlog::detail::tap
#endif	// JLOG_TAP_H
