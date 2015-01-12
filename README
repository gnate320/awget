The awget and ss executables are used to add a degree of anonymity to the wget linux command.  

The 'ss' executable is to be run on multiple network nodes to provide a 'chain-gang' of randomly accessed stepping stones for the transmittion of a file hosted on an network and retrived by wget.  The ss executables acts as "stepping stones" for completing the awget request.

The 'awget' executable is to be used with a 'chain-gang' list of "stepping stones" to obsucre the source of a wget request. 


Directions for ss:
	1.  Run with 'ss [-p port]'.  If no port is provided a port will be choosen at random.
	2.  Use ctrl+c to terminate execution.

Directions for awget:
	1.  Create a chain-gang and list each node in a "gangfile".
		a.  Run the ss executable on multiple network nodes.
		b.  List each stepping stone (instance of ss) in a text file as '<IP Address>,<Port Number>' with no spaces and each node followed by a newline.
	2.  Run with 'awget <url> [-c gangfile].
		a.  If no gangfile is specified the executable will try a default named "chaingang".
		b.  If no gangfile is specified and no file named "chaingang" exists a segmentation fault with be thrown.
		c.  The file reqested will be given the name following the last '/' character in the url requested.  If no string occurs following the last '/' character or no such character is contained in the url the requested file will be saved as "index.html".

Some issues to be aware of:
	1. The chain-gang file should not contain '<' or '>' characters.  The reading of the chaingang will fail if these characters are included.  

	2. C strings were used with no length check, therefor awget and ss may be vulnerable to buffer overflows and stack smashing attacks.

	3. Pointers were used and may not be properly deleted/freed.  Memory leaks are possible.

	4. Executables are not robust.  Excecution error will likely make all stepping stones hang and they may need to be restarted to handle the next request.

Disclaimers and Warnings:
	1.  The 'awget' and 'ss' executables were created for educational purposes during CS457 fall 2013 taught by Professor Christos Papadopoulos at Colorado State University.  They were created by and are provided by Nate Gillard, a student of the course, to display the skills learned during the course.
	2.  The student-author and professor are not responsible for thier misuse.
	3.  The student-author recommends highly that these executable should not be used for neferious purposes.  Anonymity is only increased and not guarenteed with the use of these executables.

Contact:
	Author: Nate Gillard
	Email:  nate.gillard14@alumni.colostate.edu
	
