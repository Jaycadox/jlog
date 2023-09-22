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
	this->m_LogThread = std::thread([this]() {
		while (this->m_ThreadRunning) {
			FlushQueue();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		while (FlushQueue()) {
		}
	});
}

jlog::detail::tap::AsyncTap::~AsyncTap() {
	this->m_ThreadRunning = false;
	this->m_LogThread.join();
}
bool jlog::detail::tap::AsyncTap::FlushQueue() {
	GeneralDeferedString strs[200];
	size_t count = this->m_MessageQueue.wait_dequeue_bulk_timed(strs, 200, std::chrono::milliseconds(10));

	if (count == 0) {
		return false;
	}

	std::string print_buf;

	for (auto i = 0; i < count; i++) {
		auto& str = strs[i];
		std::visit(
			[&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, DeferedString<char>>) {
					print_buf.append(arg.callback());
				} else if constexpr (std::is_same_v<T, DeferedString<wchar_t>>) {
					print_buf.append(string::WStringToString(arg.callback()));
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			},
			str);
	}

	os::Write(this->m_Descriptor,string::StringViewToByteSpan<char>(print_buf));

	return true;
}
