#pragma once
#include "core.hpp"

// Needed for cookies and queries
class Environment {
private:
	static const usize envSize = 1024 - 2;		// To align
	static const usize minElements = 64;

public:
	char* envp[envSize];
	char** optr;
	char** writePtr;

	void append(char *ptr) {
		*writePtr++ = ptr;
		*writePtr = NULL;
	}

	void reset() {
		writePtr = optr;
		*optr = NULL;
	}

	void init(char *const *envpSrc) {
		optr = envp;
		char** endPtr = envp + envSize - minElements;
		while (optr < endPtr && *envpSrc != NULL)
			*optr++ = *envpSrc++;
		*optr = NULL;
		writePtr = optr;
	}
};
