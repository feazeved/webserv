OOB Padding: 			Will not permanently modify
Clobberable padding:	Will permanently modify

POST/PRE refer to the location of the padding. [PRE] [DATA] [POST]

1) Prepending QUERY_TARGET= in cgi setup depends on the buffer being pre-clobberable-padded with 8 bytes, because QUERY_TARGET= is 13 bytes, and the minimum valid response is GET \?, which is a 6 bytes guarantee
(PRE 8 Clobberable padding)
2) Most find algorithms depend on the buffer being padded with at least 4 bytes to insert a sentinel like \r\n\r\n
(POST 8 bytes OOB padding because it is restored)
3) Match algorithms require 24 bytes OOB padding
(POST 24 bytes OOB padding)

