#pragma once
#include "core.hpp"

// Needed for cookies and queries
class Environment {
private:
	static const usize envSize = 4096 - 2;		// To align
	static const usize minElements = 64;

public:
	char* envp[envSize];
	const char** optr;
	const char** writePtr;

	void append(const char *ptr) {
		*writePtr = ptr;
		*writePtr = NULL;
	}

	void reset() {
		writePtr = optr;
		*optr = NULL;
	}

	void init(char *const *envpSrc) {
		char** endPtr = envp + envSize - minElements;
		while (optr < endPtr && *envpSrc != NULL)
			*optr++ = *envpSrc++;
		*optr = NULL;
		writePtr = optr;
	}

	Environment() : envp(), optr(), writePtr() {}
};
