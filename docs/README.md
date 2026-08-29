# webserv
*This project has been created as part of the 42 curriculum by feazeved, wlucas-f, adeimlin.*


Memory Pool A: 64MB Connection Pool
Memory Pool B: 8MB Config Pool

We use the Arena because it allows 32 byte representation

Given your requirements of

No aggregated arenas;
Span with implicit extract knowledge;
Non static connection pool inside server;

- Static variables that belong outside of the arena must 

* Arenas should perform extraction, not Span. Unless you accept the limitation that all ZSpan objects relate to one point in memory