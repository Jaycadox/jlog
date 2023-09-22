//
// Created by jayphen on 22/9/2023.
//

#ifndef JLOG_STRING_HELP_H
#define JLOG_STRING_HELP_H

#include <codecvt>
#include <locale>
#include <span>
#include <string>
#include <string_view>
#include "jlog/os.h"

namespace jlog::detail::string {

template <typename CharT>
	requires std::is_same_v<CharT, char> || std::is_same_v<CharT, wchar_t>
std::span<const uint8_t> StringViewToByteSpan(const std::basic_string_view<CharT>& str_view) {
	const auto* byteData = reinterpret_cast<const uint8_t*>(str_view.data());
	return {(const uint8_t*)byteData, (size_t)str_view.size() * sizeof(CharT)};
}

std::string WStringToString(const std::wstring& wideStr);

}  // namespace jlog::detail::string
#endif	// JLOG_STRING_HELP_H
