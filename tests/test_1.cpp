//
// Created by jayphen on 22/9/2023.
//

#include <cstdio>
#include "jlog/os.h"
#include "jlog/string_help.h"
#include "jlog/tap.h"
#include <iostream>

int main() {
	using namespace jlog::detail;
	auto stream = os::FileStream("./test.txt");
	tap::AsyncTap log(std::move(stream));


	std::cout << "start...\n";
	auto time_then = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for(int i = 0; i < 10000000; i++) {
		log.Process<char>(tap::DeferFormat<"My name is {}\n", char>("jayphen"));
	}
	auto time_now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	log.Process<char>(tap::DeferFormat<"Took {}ms\n", char>((time_now - time_then)));

	return 0;
}