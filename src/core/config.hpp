#pragma once

// Server configuration
#define HTTP_BUFFERSIZE 8192
#define MAX_VIRTUAL_SERVERS (64)	// TODO: This isn't really configurable yet
#define CONNECTION_TIMEOUT 60

#define MAX_SERVER_BLOCK_SIZE UINT16_MAX
#define MAX_LOCATION_BLOCK_SIZE INT16_MAX

#define CONFIG_POOL_SIZE (MAX_SERVER_BLOCK_SIZE * MAX_VIRTUAL_SERVERS)
#define CONNECTION_POOL_SIZE (4096ul * HTTP_BUFFERSIZE)

// Kernel configurations
#define MAX_PATH_SIZE (4096ul)
#ifdef PIPE_BUF
	#if PIPE_BUF > 4096
		#define ATOMIC_IOSIZE 4096
	#else
		#define ATOMIC_IOSIZE PIPE_BUF
	#endif
#else
	#ifdef _POSIX_PIPE_BUF
		#define ATOMIC_IOSIZE _POSIX_PIPE_BUF
	#else
		#define ATOMIC_IOSIZE 512
	#endif
#endif

#define HTTP_DIRENT_MAX_SIZE 512
