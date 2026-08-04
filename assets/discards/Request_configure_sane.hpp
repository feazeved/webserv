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
Assumption is just finished reading HTTP header and it was parsed as valid

DECODE:
	0) Read from (Client FD) into (Client Output)
NORMAL:
	1) Write from (Client Output) to (Target FD)
CHUNKED:
	1) Dechunk from (Client Output) to (Server Input)
	2) Write from (Server Input) to (Target FD)

--------------------------------------
POST:
	1) Decode from (Client FD) into (File FD)
	2*) Build a header in (Client Input)
	3) Write from (Client Input) to (Client FD)

Buffers used: Client Output, Client Input, (Server Input)
--------------------------------------
CGI: 
	1) Decode from (Client FD) into (CGI IN)
	2) Read from (CGI OUT) to (Server Output)
	3*) Parse (Server Output) until a CGI header is found:
		a) Build a header in (Client Input)
		b) Decode from (Server Output) to (Client Input)
		c) Change step 2 target (Server Output) to (Client Input)
	4) Write from (Client Input) to (Client FD)

Buffers used: Client Output, Client Input, (Server Input), Server Output
--------------------------------------
DEL:
	0) Read from (Client FD) into (Client Output)
	1*) Delete (File FD)
	2*) Build a header in (Client Input)
	3) Write from (Client Input) to (Client FD)

Buffers used: Client Output, Client Input
--------------------------------------
GET:
	0) Read from (Client FD) into (Client Output)
	1*) Build header in (Client Input)
	2) Read from (File FD) into (Client Input)
	3) Write from (Client Input) to (Client FD) 

Buffers used: Client Output, Client Input
--------------------------------------

Failure conditions are:
	- body size different than expected or greater than maxBodySize;
	- failed to write into (File FD)

*/
namespace HTTP {

	
}
