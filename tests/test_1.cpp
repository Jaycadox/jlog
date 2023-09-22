//
// Created by jayphen on 22/9/2023.
//

#include <cstdio>
#include "jlog/os.h"
#include "jlog/string_help.h"
#include "jlog/tap.h"

int main() {
	using namespace jlog::detail;
	auto stream = os::ConsoleStream();
	tap::AsyncTap log(std::move(stream));
	log.Process<char>(tap::DeferFormat<"My name is {}", char>("jayphen"));
	log.Process<char>(tap::DeferFormat<"My name is {}", char>("jayphen"));
	log.Process<char>(tap::DeferFormat<"My name is {}", char>("jayphen"));
	log.Process<char>(tap::DeferFormat<"My name is {}", char>("jayphen"));

	return 0;
}