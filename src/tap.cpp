//
// Created by jayphen on 22/9/2023.
//

#include "jlog/tap.h"
jlog::detail::tap::SyncTap::SyncTap(jlog::detail::os::Stream stream) : m_Stream(std::move(stream)) {
	std::visit([&](auto&& arg) { this->m_Descriptor = arg.GetDescriptor(); }, stream);
}

template <class>
inline constexpr bool always_false_v = false;
jlog::detail::tap::AsyncTap::AsyncTap(jlog::detail::os::Stream&& stream) : m_Stream(std::move(stream)) {
	std::visit([&](auto&& arg) { this->m_Descriptor = arg.GetDescriptor(); }, this->m_Stream);
	this->m_ThreadRunning = true;
	this->m_MessageQueue = {};
	this->m_LogThread = std::thread([this]() {
		while (this->m_ThreadRunning) {
			FlushQueue();
			std::this_thread::yield();
		}
		FlushQueue();
	});
}

jlog::detail::tap::AsyncTap::~AsyncTap() {
	this->m_ThreadRunning = false;
	this->m_LogThread.join();
}
void jlog::detail::tap::AsyncTap::FlushQueue() {
	std::queue<GeneralDeferedString> queue_cop;
	{
		std::lock_guard lg(this->m_MessageQueueMutex);
		queue_cop.swap(this->m_MessageQueue);
	}

	while (!queue_cop.empty()) {
		auto def_msg = queue_cop.front();

		std::visit(
			[&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, DeferedString<char>>) {
					os::Write(this->m_Descriptor, string::StringViewToByteSpan<char>(arg.callback()));
				} else if constexpr (std::is_same_v<T, DeferedString<wchar_t>>) {
					os::Write(this->m_Descriptor,
							  string::StringViewToByteSpan<char>(string::WStringToString(arg.callback())));
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			},
			def_msg);

		queue_cop.pop();
	}
}
