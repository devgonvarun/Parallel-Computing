# -*- Makefile -*-

#Target: Dependency
#	action

all: mandelbrot 

mandelbrot: mandelbrot.cpp
	/opt/global/gcc-7.2.0/bin/g++ -fopenmp mandelbrot.cpp -O2 -omandelbrot

clean:
	rm -f mandelbrot mandelbrot.ppm
