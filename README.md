Introduction
============
	This is a small Xwindow program to display a Mandelbrot Set implemented by
	multi-thread / multi-process programming library.

Compile
=======
	+ Dependencies:
		openmpi

	+ Compile:
		./make_all_version
		This will generate 4 versions of the program.

Usage
=====
	For Help on Command Line Arguments, `./omp --help`
	In GUI, use '='/'-' to zoom. Use 's' to save current view (read file name from stdin).
	Use 'c' to change iteration value (clearer view)
