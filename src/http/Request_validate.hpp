#include "core.hpp"


/* 

Functions: 
Something that takes the config file for the server and a request, and validates:

Function receives a file path, a buffer, IO direction (read or write), number of bytes:

1) Checks if the file path exists, if the server has permission to access it 
2) Finally return an FD or -1

*/
