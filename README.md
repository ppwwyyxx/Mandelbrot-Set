## Introduction
	This is a small Xwindow program to display a Mandelbrot Set implemented by
	multi-thread / multi-process programming library.

## Compile
	+ Dependencies:
		openmpi

	+ Compile:
		./make_all_version
		This will generate 4 versions of the program.

## Example
![demo](https://github.com/ppwwyyxx/Mandelbrot-Set/raw/master/report/res/show.png)

## Usage
	For Help on Command Line Arguments, `./omp --help`
```
$ ./omp --help
Options:
    --nproc=NUM, -n      number of threads(pthread only). number of CPUs by default
    --size=SIZE, -s      size of picture. SIZE is of format `<width>x<height>'
                         eg. `800x800' (default),
    --domain=DOMAIN, -d  plotting domain. DOMAIN is of format
                         `<xmin>,<ymin>,<width>,<height>', where x and y are
                         the coordinates of left-bottom corner.
                         eg. `-2,-2,4,4` (default).
    --iter=NUM, -i       set max number of iteration of function. 1000 by default.
    --png=FILENAME, -p   save image to png file.
    --X, -x                              use X to show image. can move and zoom image.
    --help, -h           show this help and quit
```

	In GUI, use '='/'-' to zoom. Use 's' to save current view (read file name from stdin).
	Use 'c' to change iteration value to get a clearer picture.
