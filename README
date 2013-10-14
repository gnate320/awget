Nate Gillard
CS457 Papadopoulos
p2 10/13/2013

Some issue to be aware of:


	1.  chaingang file should not contain '<' or '>' characters. otherwise the reading of the chaingang will get messed up.  

	2. Likely vulnerable to buffer over flows and stack smashing.  God, I hate C strings.

	3. I never tested command line argument for chainfile, but I think it works.

	4.  threads not joinable...

	5. possible memory leaks... damn c strings...

	6. Oh yeah and if an error occurs, it does not failt gently.  a error will likely make all SSs hang and they may need to be restarted to handle another request
	7.  Max nuber of concurrent request is set by QS_SIZE macro.

	8. Please let me know if you can't get the SSs to chian together....that should work at the least.

	9.  As of second commit everything seems to work.  Last minute phew...
