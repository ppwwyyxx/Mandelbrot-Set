// File: calculate.h
// Date: Sat Aug 25 19:32:48 2012 +0800
// Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#ifndef __HEAD__CALCULATE
#define __HEAD__CALCULATE
void colorize(int SIZE_X, int SIZE_Y);

short cal(double cr, double ci);

void cal_rectangle();

void cal_rectangle_omp();

void cal_rectangle_mpi();

void cal_rectangle_pth();

void *thread_work();

#define recv(a,b,c,d) MPI_Recv(a, b, MPI_SHORT, c, d, MPI_COMM_WORLD, &status)
#define recv_i(a,b,c,d) MPI_Recv(a, b, MPI_SHORT, c, d, MPI_COMM_WORLD, MPI_STATUS_IGNORE)
#define send(a,b,c,d) MPI_Send(a, b, MPI_SHORT, c, d, MPI_COMM_WORLD)
#define bcast(a) MPI_Bcast(&a, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD)

enum { FINISH, DATA, BEGIN, EXIT};

void _print_debug_(int line, int myid, const char* fmt, ...);

#ifdef DEBUG
#define print_debug(fmt, ...) \
	_print_debug_(__LINE__, myid, fmt, ## __VA_ARGS__)
#else
#define print_debug(fmt, ...)
#endif


#endif //HEAD
