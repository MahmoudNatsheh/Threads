// Name: Mahmoud Natsheh
// ID: 1001860023

#include "bitmap.h"
#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void * compute_image( void * arg);

//This structure holds the peramiters that we will use to create multiple threads
//These parameters of the structure Params are the previous parameters that were 
//taken in by the function in order to run one thread
struct Params
{
	struct bitmap *bm;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int thread_id;
	int NUM_THREADS;
};
//function that will print the diffent options the user can use
//This will print the mauel when the user inputs 
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-n <number of threads>  set number of threads to run. (default=1)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	//will hold the time the program started and ended, this will be used later
	//to measure how long it took to run the program with x amout of treads
	struct timeval begin_time;
	struct timeval end_time;
	//collects when the program started
	gettimeofday(&begin_time, NULL);
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	//Deafult will run with one thread

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;
	int NUM_THREADS = 1;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:h"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				NUM_THREADS = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s NUM_THREADS = %d\n",xcenter,ycenter,scale,max,outfile,NUM_THREADS);

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm,MAKE_RGBA(0,0,255,0));

	int i;

	//creates NUM_THREDS amount of threads to run the proram, NUM_THREADS is taken in from the user earlier in the program
	pthread_t tid[NUM_THREADS];
	//creates a structure with NUM_THREADS amout of spaces in the structure
	struct Params params[NUM_THREADS];
	
	//This will run NUM_THREADS times and create the number of threads to run the multithreading in the program
	//it will five each thread its taks and parameters it needs
	for(i = 0; i < NUM_THREADS; i++)
	{
		params[i].bm = bm;
		params[i].xmin = xcenter-scale;
		params[i].xmax = xcenter+scale;
		params[i].ymin = ycenter-scale;
		params[i].ymax = ycenter+scale;
		params[i].max = max;
		params[i].thread_id = i;
		params[i].NUM_THREADS = NUM_THREADS;

		pthread_create(&tid[i], NULL, compute_image, (void *)&params[i]);
	}
	//once the program has completed the mutithreading, we will join all parts of the image that each thread created
	//in order to have one full image to out put to the user
	for(i = 0; i < NUM_THREADS; i++){
		pthread_join(tid[i], NULL);
	}
	// Compute the Mandelbrot image
	// compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max);

	// Save the image in the stated file.
	if(!bitmap_save(bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}
	//this will get the time the program completed in order to compute how long it took to execute
	gettimeofday(&end_time, NULL);
	//we will compute how long it took to run the program in microseconds using the end time - start time
	long time_to_execute = ( end_time.tv_sec * 1000000 + end_time.tv_usec ) -
						( begin_time.tv_sec * 1000000 + begin_time.tv_usec );
	//Output to user how long it took to execute the program
	printf("This code took %ld microseconds to execute\n", time_to_execute);
	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

// the parameter we will take in is a void * since we will accept a structure that will
//tell each tread what to run
void * compute_image( void * arg )
{
	//take in the parameters passed in by the structure that will tell this thread what to run in the function
	struct Params *params = (struct Params *)arg;

	//we will put each parameter recived from Params structure into a variable to use throught the function
	struct bitmap *bm = params-> bm;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int NUM_THREADS;

	xmin = params->xmin;
	xmax = params->xmax;
	ymin = params->ymin;
	ymax = params->ymax;
	max = params->max;
	NUM_THREADS = params->NUM_THREADS;

	int i,j;

	int width = bitmap_width(bm);
	int height = bitmap_height(bm);

	// For every pixel in the image...

	int start_row;
	int end_row;

	//so that the thread know when to end we will save its start and end into varables
	start_row = params->thread_id * height / NUM_THREADS;
	end_row = (start_row + height / NUM_THREADS);
	//ths tread will run the amout of rows it has to using the sart and end row variables
	for(j= start_row;j<end_row;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			bitmap_set(bm,i,j,iters);
		}
	}
	return NULL;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{

	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}



