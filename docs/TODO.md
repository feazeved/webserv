1. Ensure all fields being stored in parsing are no longer than 4096 bytes

2. Ensure S_ISDIR check is in a common helper (uploadStore parserlocations)

3. Possibly make s_length_check print the error line

4. Fix default error pages using epoch

5. Fix processing of CGI block

6. Revise Arena starting size

