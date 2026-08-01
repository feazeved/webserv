#pragma once

#include <unistd.h>
#include <sys/epoll.h>
#include "core.hpp"
#include "http/Request.hpp"

/*
For post requests, could write into a TMP file instead of overwriting
Then once it is complete, DEL and MOVE

Read from, write to:   FD
Read to,   write from: Buffer
*/

/* 

DECODE NORMAL:
	1) Write from (Client Output) to (Target FD)
DECODE CHUNKED:
	1) Dechunk from (Client Output) to (Stack Scratch Buffer)
	2) Write from (Stack Scratch Buffer) to (Target FD)
	3) Copy remaining bytes (if any) to client input and compact

ENTRY POINTS:
	0) Read from (Client FD) into (Client Output)

EXIT POINTS:
	END) Write from (Client Input) to (Client FD)

Assumption is just finished reading HTTP header and it was parsed as valid

--------------------------------------
POST:
	1)	Decode from (Client FD) into (File FD)
	2*)	Build a header in (Client Input)

--------------------------------------
CGI: 
	1*)	Change (Client Output) offset by 4096
	2)	Decode from (Client FD) into (CGI IN)
	3)	Read from (CGI OUT) to (Client Output)
	4*)	Prepend the header to (Client Output) once CGI header is found

--------------------------------------
DEL:
	1*)	Delete (File FD)
	2*)	Build a header in (Client Input)

--------------------------------------
GET:
	1*)	Build header in (Client Input)
	2)	Read from (File FD) into (Client Input) 

--------------------------------------

Failure conditions are:
	- body size different than expected or greater than maxBodySize;
	- failed to write into (File FD)
	- failed to del or get file

*/
namespace HTTP {

	
}
