## Introduction

This is a small Xwindow program to display a [Mandelbrot Set](http://en.wikipedia.org/wiki/Mandelbrot_set) implemented by multi-thread / multi-process programming library.

## Compile
Dependencies: [openmpi](http://www.open-mpi.org/)

Compile:
```
$ ./make_all_version
```
This will generate 4 versions of the program: sequential, openmp, mpi, pthread

## Example
![demo](https://github.com/ppwwyyxx/Mandelbrot-Set/raw/master/report/res/show.png)

## Usage
See Help on Command Line Arguments, e.g.:

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

In GUI, use '='/'-' to zoom. Use 's' to save current view (file name is read from stdin).

Use 'c' to change iteration value to get a clearer picture.
