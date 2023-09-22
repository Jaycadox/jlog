//
// Created by jayphen on 22/9/2023.
//

#include "jlog/string_help.h"

std::string jlog::detail::string::WStringToString(const std::wstring& wideStr) {
#ifdef JLOG_WINDOWS
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (bufferSize == 0) {
		std::cerr << "Error in WideCharToMultiByte" << std::endl;
		return "";
	}

	std::string narrowStr(bufferSize - 1, 0);  // Exclude the null-terminator
	if (WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &narrowStr[0], bufferSize, nullptr, nullptr) == 0) {
		std::cerr << "Error in WideCharToMultiByte" << std::endl;
		return "";
	}

	return narrowStr;
#elifdef JLOG_UNIX_LIKE
	// Determine the required buffer size for the narrow string
	size_t bufferSize = wcstombs(nullptr, wideStr.c_str(), 0);
	if (bufferSize == static_cast<size_t>(-1)) {
		return "";
	}

	// Allocate a buffer for the narrow string
	auto narrowBuffer = std::unique_ptr<char[]>(new char[bufferSize + 1]);	// +1 for null-terminator

	// Convert the wide string to a narrow string
	if (wcstombs(narrowBuffer.get(), wideStr.c_str(), bufferSize + 1) == static_cast<size_t>(-1)) {
		return "";
	}

	// Null-terminate the narrow string
	narrowBuffer[bufferSize] = '\0';

	// Create a std::string from the narrow string
	std::string narrowStr(narrowBuffer.get());

	return narrowStr;
#endif
}
