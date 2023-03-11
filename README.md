# ShiftingSands

To compile and run in a UNIX-like environment, first navigate to the directory where the root folder for the project is, using "ls" to view the files in the current directory and "cd <directory>" to change directories. Once you're in the project's root directory, you should be able to see CMakeLists.txt when you use the "ls" command. Now you should be able to run cmake with this command:
	
	cmake -B build
	
which will create a new directory called "build". Navigate to this directory with "cd build", and then simply execute the command:
	
	make
	
which will link and compile the program into an executable. The executable file should be called "589-689-3D-skeleton" (would be nice if we could modify this name into something shorter, but that may involve editing CMakeLists.txt). Run the program with this command:
	
	./589-689-3D-skeleton
