// File: calculate.cc
// Date: Mon Sep 02 15:44:54 2013 +0800
// Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <omp.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdarg>
#include "calculate.h"
#include "Xoutput.h"

using namespace std;

extern int maxiter;
extern double xmin, ymin, xlen, ylen;
extern int SIZE_X, SIZE_Y;
extern short* img;
extern int myid, nproc;

int unit = 50; // num of lines of data to be calculated in each task

pthread_mutex_t nowx_mutex = PTHREAD_MUTEX_INITIALIZER;
int p_nowx = 0;

void _print_debug_(int line, int myid, const char* fmt, ...){
	fprintf(stderr, "[proc %d line %d] ", myid, line);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

short cal(double cr, double ci){
	double zr = cr, zi = ci,
		   tr = zr * zr, ti = zi * zi;
	int k = maxiter;
	/*
	 *if (tr + ti < 1/16) return 255;
	 */
	for (; (tr + ti < 4) && (k > 0); k --){
		zi = 2 * zr * zi + ci;
		zr = tr - ti + cr;
		tr = zr * zr;
		ti = zi * zi;
	}
	if (!k ) return 255;
	return (maxiter - k) * 230 / (maxiter - 1);
}


#define cal_pixel(ptr, line_start,  i, j) \
	do { \
		double cr = line_start * xlen / SIZE_X + xmin; \
		double ci = j * ylen / SIZE_Y + ymin; \
		short t = cal(cr, ci); \
		ptr[i * SIZE_Y + j] = 255 - sqrt(t) * 16;\
	} while (0);

void cal_rectangle_omp(){
#ifdef USE_OMP
#pragma omp parallel for schedule(dynamic)
#endif
	for (int i = 0; i < SIZE_X; i ++)
		for (int j = 0; j < SIZE_Y; j ++)
			cal_pixel(img, i, i, j);
}

void cal_rectangle_mpi(){
	unit = 100;
	MPI_Status status;

	if (myid == 0){
		// the line[nowy] are to be assigned (nowy can == SIZE_Y - 1)
		int nFinished = 0;
		short nowx = 0;
		short buf;
		for (int i = 1; i < nproc; i ++){
			// nowy is the beginning line in the task
			send(&nowx, 1, i, BEGIN);
			print_debug("send task %d", nowx);
			int line_of_work = min(unit, SIZE_X - nowx);
			nowx += line_of_work;
			if (nowx >= SIZE_X)
				break;
		}
		while ((nowx < SIZE_X) || (nFinished < nproc - 1)){
			print_debug("nowx: %d, nFinished: %d", nowx, nFinished);
			// buf is the beginning line in the task ( 0 ~ SIZE_Y - 1)
			recv(&buf, 1, MPI_ANY_SOURCE, FINISH);
			int source = status.MPI_SOURCE;
			print_debug("get finished task %d from %d", buf, source);
			nFinished ++;
			int line_of_work = min(unit, SIZE_X - buf);
			//recv data
			recv_i(&img[buf * SIZE_Y] , line_of_work * SIZE_Y, source, DATA);
			print_debug("get finished data from %d", source);

			if (nowx < SIZE_X){
				nFinished --;
				// buf is the beginning line in the task
				print_debug("try send task %d to %d", nowx, source);
				send(&nowx, 1, source, BEGIN);
				print_debug("finish send task %d to %d", nowx, source);
				int line_of_work = min(unit, SIZE_X - nowx);
				nowx += line_of_work;
			}
		}
		// send exit signal
		for (int i = 1; i < nproc; i ++){
			send(&buf, 1, i, EXIT);
			print_debug("sent exit signal");
		}
	} else {
		bool done = false;
		while (!done){
			short buf;
			recv(&buf, 1, 0, MPI_ANY_TAG);
			if (status.MPI_TAG == BEGIN){
				print_debug("get task %d", buf);
				int line_of_work = min(unit, SIZE_X - buf);

				short* line;
				line = new short [line_of_work * SIZE_Y];
				/*
				 *short line[20000];
				 */
				for (int i = 0; i < line_of_work; i ++)
					for (int j = 0; j < SIZE_Y; j ++){
						int now_line = i + buf;
						cal_pixel(line, now_line, i, j);
						/*
						 *line[i * SIZE_Y + j] = 1;
						 */
					}

				send(&buf, 1, 0, FINISH);
				print_debug("send finished signal for task %d", buf);
				send(line, line_of_work * SIZE_Y, 0, DATA);
				print_debug("send data to root for task %d", buf);

				delete[] line;
				print_debug("finish delete");
			}
			else if (status.MPI_TAG == EXIT){
				done = true;
				print_debug("get exit signal");
			}
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	print_debug("exit");
	return ;
}


void *thread_work(void*){
	int now_line, line_of_work;
	while (1){
		pthread_mutex_lock(&nowx_mutex);
		if (p_nowx >= SIZE_X) break;
		now_line = p_nowx;
		p_nowx = min(p_nowx + unit, SIZE_X);
		line_of_work = p_nowx - now_line;
		pthread_mutex_unlock(&nowx_mutex);

		for (int i = now_line; i < now_line + line_of_work; i ++)
			for (int j = 0; j < SIZE_Y; j ++){
				cal_pixel(img, i, i, j);
			}
	}
	pthread_mutex_unlock(&nowx_mutex);
	pthread_exit(NULL);
}

void cal_rectangle_pth(){
	unit = 10;
	pthread_t threads[nproc];
	p_nowx = 0;
	for (int t = 0; t < nproc; t ++){
		int ret = pthread_create(&threads[t], NULL, thread_work, (void*)NULL);
		if (ret){
			cerr << "ret from pthread_create error";
		}
	}
	void* status;
	for (int t = 0; t < nproc; t ++)
		pthread_join(threads[t], &status);
}


#undef cal_pixel
