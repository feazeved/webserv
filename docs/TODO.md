1. Ensure all fields being stored in parsing are no longer than 4096 bytes	(Done?)

2. Possibly make s_length_check print the error line

3. Make a better string view class, it is super annoying to use

4. Validate the CGI interpreters (check for access)

5. Do more target validation inside check_location, it is too permissive

6. Check stat functions vs fstat

7. Make multi-level bitarray (see how to do specializations)

8. Null terminate CGI Blocks

9. Maybe reduce the allocation alignment to 16 bytes, cache locality might be hurt

10. Configure signal handling

11. Fix the connection pool memory layout
Connection cannot have constructors, given that they are not supposed to fault pages
The find time out function is also incorrect, both in ownership and infinite loop for healthy connections

12. Fix the epoll API design
Connections will handle their ingress and removal from epoll
Check if the error checking is necessary for epoll

13. Change timeout design

14. Verify the close on exec

15. Verify VirtualServer memory layout, and whether it requires a constructor
Specifically the parts regarding init(). It only gets called once, it shouldnt need checks
Same for Locations struct

16. Verify exit strategies for PERR_EXIT

17. Create the file handling for query target and read file