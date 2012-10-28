// File: main.cc
// Date: Sun Aug 26 22:17:34 2012 +0800
// Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <getopt.h>
#include <cassert>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <omp.h>

#include "pngwriter.h"
#include "calculate.h"
#include "Xoutput.h"
using namespace std;

int SIZE_X = 800, SIZE_Y = 800;
short *img;
double xmin = -2, ymin = -2, xlen = 4, ylen = 4;
int maxiter = 1000;
int myid, nproc;

bool use_png = false;
int use_x = 0;
string png_fname = "a.png";

void cal_rectangle(){
#ifdef USE_PTHREAD
	cal_rectangle_pth();
#else
#ifdef USE_MPI
	cal_rectangle_mpi();
#else
	cal_rectangle_omp();
#endif
#endif
	return;
}

#define error_exit(fmt, ...) \
	do { \
		static const int MSG_LEN_MAX = 1000; \
		static char msg[MSG_LEN_MAX + 1]; \
		sprintf(msg, "error : " fmt, ##__VA_ARGS__); \
		fprintf(stderr, "%s\n", msg); \
		print_help(); \
	} while (0)

void print_help(){
	printf("Options:\n");
	printf("    --nproc=NUM, -n      number of threads(pthread only). number of CPUs by default\n");
	printf("    --size=SIZE, -s      size of picture. SIZE is of format `<width>x<height>'\n");
	printf("                         eg. `800x800' (default), \n");
	printf("    --domain=DOMAIN, -d  plotting domain. DOMAIN is of format \n");
	printf("                         `<xmin>,<ymin>,<width>,<height>', where x and y are \n");
	printf("                         the coordinates of left-bottom corner.\n");
	printf("                         eg. `-2,-2,4,4` (default).\n");
	printf("    --iter=NUM, -i       set max number of iteration of function. 1000 by default.\n");
	printf("    --png=FILENAME, -p	 save image to png file.\n");
	printf("    --X, -x				 use X to show image. can move and zoom image.\n");
	printf("    --help, -h           show this help and quit\n");
	exit(0);
}

void parse_args(int argc, char* argv[]){
	if (argc == 1)
		print_help();
	option long_options[] = {
		{"nproc",		required_argument,	NULL, 'n'},
		{"size",		required_argument,	NULL, 's'},
		{"domain",		required_argument,	NULL, 'd'},
		{"show",		required_argument,	NULL, 'w'},
		{"iter",		required_argument,	NULL, 'i'},
		{"png",			required_argument,	NULL, 'p'},
		{"help",		no_argument,		NULL, 'h'},
		{"X",			no_argument,		NULL, 'x'},
	};
	int opt;
	while ((opt = getopt_long(argc, argv, "-n:s:d:w:i:hp:x", long_options, NULL)) != -1) {
		switch (opt) {
			case 'h':
				print_help();
				exit(0);
			case 'p':
				use_png = true;
				png_fname = optarg;
				break;
			case 'x':
				use_x = 1;
				break;
			case 'i':
				maxiter = atoi(optarg);
				maxiter = max(maxiter, 1);
				break;
			case 'n':
				nproc = atoi(optarg);
				nproc = max(nproc, 1);
				break;
			case 's':
				{
					int w, h;
					if (sscanf(optarg, "%dx%d", &w, &h) != 2)
						error_exit("invalid size `%s'", optarg);
					SIZE_X = w, SIZE_Y = h;
				}
				break;
			case 'd':
				{
					double x, y, w, h;
					if (sscanf(optarg, "%lf,%lf,%lf,%lf", &x, &y, &w, &h) != 4)
						error_exit("invalid domain `%s'", optarg);
					if ((w <= 0) || (h <= 0))
						error_exit("invalid domain `%s'", optarg);
					xmin = x, ymin = y, xlen = w, ylen = h;
				}
				break;
			case '1':
			case '?':
			default:
				print_help();
				break;
		}
	}
}

int main(int argc, char* argv[]){
	double t1 = 0, t2;

	nproc = sysconf(_SC_NPROCESSORS_ONLN);
	MPI_Init(&argc, &argv);
#ifdef USE_MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	if (myid == 0){
#endif
		parse_args(argc, argv);
		img = new short [SIZE_X * SIZE_Y];

		t1 = MPI_Wtime();
#ifdef USE_MPI
	}
	MPI_Comm_size(MPI_COMM_WORLD, &nproc); // nproc was used before in parse_args
	if (nproc == 1){
		cerr << "You Need At Least 2 Processors To Run This Program !"  << endl;
		exit(1);
	}
	MPI_Bcast(&SIZE_X, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&SIZE_Y, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&maxiter, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&use_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
	bcast(xmin);bcast(ymin);bcast(xlen);bcast(ylen);
#endif

	cal_rectangle();
#ifdef USE_MPI
	if (myid == 0){
#endif
		t2 = MPI_Wtime();
		printf("%.6lf seconds elapsed\n", t2 - t1);  

		if (use_png)
			write_image(png_fname.c_str(), img, SIZE_X, SIZE_Y);

		if (use_x){
			initialized_window();
			X_drawimg();
		}
#ifdef USE_MPI
	}
#endif
	if (use_x)
		X_wait();
	delete[] img;
	MPI_Finalize();
	return 0;
}
