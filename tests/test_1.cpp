//
// Created by jayphen on 22/9/2023.
//

#include <cstdio>
#include "jlog/log.h"
#include <iostream>

int main() {

	jlog::Logger log;
	log.AddFile<jlog::detail::tap::SyncTap>("./latest.log");
	log.AddConsole();
	log.Info<"Welcome!">();

	return 0;
}