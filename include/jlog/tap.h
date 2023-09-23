//
// Created by jayphen on 22/9/2023.
//

#ifndef JLOG_TAP_H
#define JLOG_TAP_H

#include <blockingconcurrentqueue.h>
#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <jlog/os.h>
#include <atomic>
#include <functional>
#include <queue>
#include <thread>
#include "jlog/string_help.h"

namespace jlog {
enum class LogSeverity : uint8_t { Info, Warn, Error, Fatal, Debug };
}

namespace jlog::detail::tap {

template <typename T>
using LogCallback = std::function<std::basic_string<T>(void)>;
using LogTimestamp = std::chrono::time_point<std::chrono::system_clock>;

template <typename T = char>
struct DeferredLog {
	using Type = T;
	LogCallback<T> m_Callback;
	LogTimestamp m_Timestamp;
	LogSeverity m_Severity;
};

class DefaultFormatter {
   private:
	static std::string_view GetSeverityName(LogSeverity severity) {
		switch (severity) {
			case LogSeverity::Info:
				return "Info";
			case LogSeverity::Warn:
				return "Warn";
			case LogSeverity::Error:
				return "Error";
			case LogSeverity::Fatal:
				return "Fatal";
			case LogSeverity::Debug:
				return "Debug";
		}
		std::unreachable();
	}

   public:
	static auto Perform(std::string_view str, LogTimestamp timestamp, LogSeverity severity) {
		return fmt::format("[{:%H:%M:%S}] [{}] {}\n", timestamp, GetSeverityName(severity), str);
	}
};

using GeneralDeferedLog = std::variant<DeferredLog<char>, DeferredLog<wchar_t>>;

template <typename T, size_t N>
struct StringLiteral {
	constexpr StringLiteral(const T (&str)[N]) { std::copy_n(str, N, value); }
	T value[N];
};

template <LogSeverity Sev, StringLiteral Str, typename T = char, typename... Args>
auto DeferFormat(Args&&... args) {
	return DeferredLog<T>{[args...]() { return fmt::format(Str.value, args...); }, std::chrono::system_clock::now(),
						  Sev};
}

class SyncTap {
	os::Stream m_Stream;
	os::StreamDescriptor m_Descriptor;

   public:
	explicit SyncTap(os::Stream&& stream);
	SyncTap(SyncTap&& other) noexcept = default;
	SyncTap& operator=(SyncTap&& other) noexcept = default;

	template <typename Format>
	void Start() {}

	template <typename Format, typename T>
	void Process(DeferredLog<T>& str) {
		const auto formatted_string = str.m_Callback();
		os::Write(this->m_Descriptor, string::StringViewToByteSpan<T>(formatted_string));
	}
};

class AsyncTap {
	// TODO: make atomic
	std::unique_ptr<std::atomic_bool> m_ThreadRunning;
	std::thread m_LogThread;
	moodycamel::BlockingConcurrentQueue<GeneralDeferedLog> m_MessageQueue;
	os::Stream m_Stream;
	os::StreamDescriptor m_Descriptor;

   public:
	explicit AsyncTap(os::Stream&& stream);
	AsyncTap(AsyncTap&& other) noexcept = default;
	AsyncTap& operator=(AsyncTap&& other) noexcept = default;
	~AsyncTap();

	template <typename Format>
	void Start();

	template <typename Format, typename T>
	void Process(DeferredLog<T> str) {
		m_MessageQueue.enqueue(str);
	}

   private:
	template <typename Format>
	bool FlushQueue(bool expedite);
};

using GenericTap = std::variant<SyncTap, AsyncTap>;

}  // namespace jlog::detail::tap

jlog::detail::tap::SyncTap::SyncTap(os::Stream&& stream) : m_Stream(std::move(stream)) {
	std::visit([&](auto& arg) { this->m_Descriptor = arg.GetDescriptor(); }, m_Stream);
}

template <class>
inline constexpr bool always_false_v = false;

jlog::detail::tap::AsyncTap::AsyncTap(os::Stream&& stream) : m_Stream(std::move(stream)) {
	std::visit([&](auto&& arg) { this->m_Descriptor = arg.GetDescriptor(); }, this->m_Stream);
	this->m_ThreadRunning = std::make_unique<std::atomic_bool>(false);
}

jlog::detail::tap::AsyncTap::~AsyncTap() {
	if (this->m_ThreadRunning.get()) {
		auto old = (*this->m_ThreadRunning).load();
		*this->m_ThreadRunning = false;
		this->m_LogThread.join();
	}
}

template <typename Format>
bool jlog::detail::tap::AsyncTap::FlushQueue(const bool expedite) {
	GeneralDeferedLog strs[300];
	size_t count;
	if (expedite) {
		count = this->m_MessageQueue.wait_dequeue_bulk_timed(strs, 300, std::chrono::milliseconds(10));
	} else {
		count = this->m_MessageQueue.wait_dequeue_bulk_timed(strs, 300, std::chrono::milliseconds(10));
	}

	if (count == 0) {
		return false;
	}

	std::string print_buf;

	for (auto i = 0; i < count; i++) {
		auto& str = strs[i];
		std::string new_str;
		LogTimestamp timestamp;
		LogSeverity severity;

		std::visit(
			[&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, DeferredLog<char>>) {
					new_str = arg.m_Callback();
					timestamp = arg.m_Timestamp;
					severity = arg.m_Severity;
				} else if constexpr (std::is_same_v<T, DeferredLog<wchar_t>>) {
					new_str = string::WStringToString(arg.m_Callback());
					timestamp = arg.m_Timestamp;
					severity = arg.m_Severity;
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			},
			str);

		print_buf.append(Format::Perform(new_str, timestamp, severity));
	}

	os::Write(this->m_Descriptor, string::StringViewToByteSpan<char>(print_buf));

	return true;
}
template <typename Format>
void jlog::detail::tap::AsyncTap::Start() {
	*this->m_ThreadRunning = true;
	this->m_LogThread = std::thread([this]() {
		while (this->m_ThreadRunning.get() && (*this->m_ThreadRunning).load()) {
			FlushQueue<Format>(false);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		while (FlushQueue<Format>(true)) {
		}
	});
}

#endif	// JLOG_TAP_H
