#pragma once
#include "core.hpp"

// Needed for cookies and queries
class Environment {
private:
	static const usize envSize = 1024;
	static const usize minElements = 64;
	Environment();

public:
	static char* envp[envSize];
	static char** optr;
	static char** writePtr;

	static void append(char *ptr) {
		*writePtr++ = ptr;
		*writePtr = NULL;
	}

	static void reset() {
		writePtr = optr;
		*optr = NULL;
	}

	static void init(char *const *envpSrc) {
		optr = envp;
		char** endPtr = envp + envSize - minElements;
		while (optr < endPtr && *envpSrc != NULL)
			*optr++ = *envpSrc++;
		*optr = NULL;
		writePtr = optr;
	}
};
