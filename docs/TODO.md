1. Ensure all fields being stored in parsing are no longer than 4096 bytes	(Done?)

2. Possibly make s_length_check print the error line

3. Make a better string view class, it is super annoying to use

4. Validate the CGI interpreters (check for access)

5. Do more target validation inside check_location, it is too permissive

6. Check stat functions vs fstat

7. Make multi-level bitarray (see how to do specializations)

8. Null terminate CGI Blocks