//
// Created by jayphen on 23/9/2023.
//

#ifndef JLOG_LOG_H
#define JLOG_LOG_H

#include <list>
#include "jlog/tap.h"

namespace jlog {

using DefaultFormatter = detail::tap::DefaultFormatter;

using GenericTap = detail::tap::GenericTap;

using SyncTap = detail::tap::SyncTap;
using AsyncTap = detail::tap::AsyncTap;

template <typename Format = DefaultFormatter, size_t NumTaps = 128>
class Logger {
	std::vector<GenericTap> m_Taps;

   public:
	Logger();
	template <typename T = jlog::detail::tap::AsyncTap>
	bool AddConsole();

	template <typename T = jlog::detail::tap::AsyncTap>
	bool AddFile(std::filesystem::path&& path);

	template <LogSeverity Sev, detail::tap::StringLiteral Str, typename T = char, typename... Args>
	auto Log(Args&&... args) {
		auto log = detail::tap::DeferFormat<Sev, Str, T>(args...);
		for (auto& tap : this->m_Taps) {
			// TODO: m_Callback() is being called multiple times
			std::visit([&log](auto& ta) { ta.template Process<Format>(log); }, tap);
		}
	}

	template <detail::tap::StringLiteral Str, typename T = char, typename... Args>
	auto Info(Args&&... args) {
		return Log<LogSeverity::Info, Str, T, Args...>(std::move(args)...);
	}

	template <detail::tap::StringLiteral Str, typename T = char, typename... Args>
	auto Warn(Args&&... args) {
		return Log<LogSeverity::Warn, Str, T, Args...>(std::move(args)...);
	}

	template <detail::tap::StringLiteral Str, typename T = char, typename... Args>
	auto Error(Args&&... args) {
		return Log<LogSeverity::Error, Str, T, Args...>(std::move(args)...);
	}

	template <detail::tap::StringLiteral Str, typename T = char, typename... Args>
	auto Fatal(Args&&... args) {
		return Log<LogSeverity::Fatal, Str, T, Args...>(std::move(args)...);
	}

	template <detail::tap::StringLiteral Str, typename T = char, typename... Args>
	auto Debug(Args&&... args) {
		return Log<LogSeverity::Debug, Str, T, Args...>(std::move(args)...);
	}
};
}  // namespace jlog

template <typename Format, size_t NumTaps>
jlog::Logger<Format, NumTaps>::Logger() {
	m_Taps.reserve(NumTaps);
}

template <typename Format, size_t NumTaps>
template <typename T>
bool jlog::Logger<Format, NumTaps>::AddConsole() {
	m_Taps.emplace_back(std::move(T(std::move(detail::os::ConsoleStream()))));
	std::visit([](auto& tap) { tap.template Start<Format>(); }, this->m_Taps.back());
	return true;
}

template <typename Format, size_t NumTaps>
template <typename T>
bool jlog::Logger<Format, NumTaps>::AddFile(std::filesystem::path&& path) {
	m_Taps.emplace_back(std::move(T(std::move(detail::os::FileStream(std::move(path))))));
	std::visit([](auto& tap) { tap.template Start<Format>(); }, this->m_Taps.back());
	return true;
}
#endif	// JLOG_LOG_H
