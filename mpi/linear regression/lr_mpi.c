#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

/******************************************************************************
 
 * To compile:
 *   mpicc -o lr_mpi lr_mpi.c -lm
 
 * To run:
 *   mpirun -n 9 ./lr_mpi
 
 *****************************************************************************/

typedef struct point_t {
  double x;
  double y;
} point_t;

int n_data = 1000;
point_t data[];

double residual_error(double x, double y, double m, double c) {
  double e = (m * x) + c - y;
  return e * e;
}

double rms_error(double m, double c) {
  int i;
  double mean;
  double error_sum = 0;
  
  for(i=0; i<n_data; i++) {
    error_sum += residual_error(data[i].x, data[i].y, m, c);  
  }
  
  mean = error_sum / n_data;
  
  return sqrt(mean);
}

int timedifference(struct timespec *start, struct timespec *finish, long long int *difference) {
   long long int dsec = finish->tv_sec - start->tv_sec;
   long long int dnsec = finish->tv_nsec - start->tv_nsec;

	if(dnsec < 0) {
		dsec--;
		dnsec += 1000000000; 
	}

   *difference = dsec * 1000000000 + dnsec;
   return !(*difference > 0);
}

int main() {

  struct timespec start, finish;
  long long int timeelapsed;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int rank, size;
  int i;
  double bm = 1.3;
  double bc = 10;
  double be;
  double dm[8];
  double dc[8];
  double e[8];
  double step = 0.01;
  double best_error = 999999999;
  int best_error_i;
  int minimum_found = 0;
  double pError = 0;
  double baseMC[2];
  
  double om[] = {0,1,1, 1, 0,-1,-1,-1};
  double oc[] = {1,1,0,-1,-1,-1, 0, 1};

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  be = rms_error(bm, bc);

  if(size != 9) {
    if(rank == 0) {
      printf("This program needs to run on exactly 9 processes\n");
      return 0;
     }
   } 

  while(!minimum_found)
  {
    if (rank != 0)
	{
		i = rank -1;
		dm[i] = bm + (om[i] * step);
		dc[i] = bc + (oc[i] * step);
		pError = rms_error (dm[i], dc[i]);

		MPI_Send (&pError, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send (&dm[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send (&dc[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

		MPI_Recv (&bm, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv (&bc, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv (&minimum_found, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
    else
	{
		for(i = 1; i < size; i++)
		{
			MPI_Recv (&pError, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv (&dm[i-1] , 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv (&dc[i-1], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			if(pError < best_error)
			{
				best_error = pError;
				best_error_i = i - 1;
			}
		}

		if(best_error < be)
		{
			be = best_error;
			bm = dm[best_error_i];
			bc = dc[best_error_i];
		}
		
		else
		{
			minimum_found = 1;
		}
		
		for(i = 1; i < size; i++)
		{
			MPI_Send (&bm, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			MPI_Send (&bc, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			MPI_Send (&minimum_found, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

		}
	}
    }

    if(rank == 0)
      {
	printf ("minimum m,c is %lf, %lf with error %lf\n", bm, bc, be);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	timedifference(&start, &finish, &timeelapsed);
	printf("TIme elasped: %lldnsec or %0.9lfsec\n", timeelapsed, (timeelapsed/1.0e9));
      }

  MPI_Finalize();
  return 0;
}

point_t data[] = {
  {87.72,160.36},{79.77,154.56},{74.88,144.94},{67.05,157.02},
  {73.75,121.36},{69.58,132.51},{67.21,143.57},{72.13,141.31},
  {87.64,144.77},{65.42,146.46},{83.09,144.82},{73.30,137.10},
  {76.75,151.67},{69.17,144.50},{87.61,171.62},{83.56,164.97},
  {75.57,137.55},{73.80,156.25},{75.75,130.90},{97.13,193.14},
  { 6.54,74.00},{54.31,100.05},{57.36,127.37},{93.54,172.18},
  {98.26,190.44},{87.27,149.69},{66.44,143.27},{98.24,171.56},
  {27.74,98.27},{10.51,57.95},{88.17,179.14},{22.66,63.64},
  {30.46,64.81},{65.24,135.15},{ 1.71,39.30},{91.29,171.76},
  {77.43,153.66},{68.29,132.07},{33.43,82.08},{60.40,119.17},
  {15.86,64.98},{61.96,126.18},{11.05,58.97},{23.99,67.74},
  {21.13,65.78},{34.79,97.87},{22.32,74.86},{78.29,136.48},
  {25.38,84.45},{94.49,169.91},{35.62,99.82},{58.20,127.53},
  {98.37,182.22},{93.79,168.74},{26.98,91.60},{30.55,103.77},
  { 5.86,44.79},{78.96,159.17},{93.19,158.82},{24.73,51.92},
  {94.91,186.12},{61.84,135.07},{ 2.60,37.48},{95.93,189.32},
  {95.39,157.13},{20.24,69.22},{29.93,71.36},{68.29,148.83},
  {36.68,92.16},{93.29,180.89},{30.53,75.37},{48.10,120.04},
  {83.17,153.60},{ 8.22,50.63},{ 8.76,50.80},{97.71,171.61},
  {13.28,46.71},{ 1.07,43.74},{11.16,71.21},{20.98,78.43},
  {21.89,50.58},{11.51,55.46},{ 3.22,43.94},{41.33,86.40},
  { 9.72,66.88},{10.91,51.56},{40.75,100.96},{81.92,161.87},
  {54.44,112.70},{11.35,62.04},{69.49,123.86},{50.51,96.47},
  {74.84,150.85},{77.35,150.52},{81.72,163.59},{11.65,37.53},
  {13.48,52.63},{79.02,166.24},{84.93,171.49},{59.13,117.54},
  {70.14,129.94},{58.90,118.01},{28.64,107.03},{43.71,112.86},
  {58.86,114.54},{78.67,160.78},{94.74,171.99},{70.31,122.40},
  {16.20,74.80},{58.59,110.98},{70.81,130.09},{ 7.86,46.69},
  {91.46,153.64},{34.05,87.81},{24.06,68.39},{66.23,134.24},
  {58.69,121.12},{ 0.18,38.19},{20.48,63.34},{63.79,125.30},
  {27.38,80.17},{39.12,100.73},{85.45,151.11},{48.89,92.37},
  {81.54,155.19},{20.52,55.22},{16.33,56.39},{82.69,158.63},
  {78.03,160.98},{ 3.36,62.70},{ 9.16,36.34},{ 5.56,50.25},
  {96.94,185.14},{46.54,107.56},{13.32,54.76},{21.63,58.16},
  {35.36,86.17},{92.63,175.36},{35.77,93.56},{24.99,73.63},
  {78.27,153.33},{44.20,89.01},{ 8.41,28.53},{22.39,65.89},
  { 4.01,63.33},{61.57,131.79},{39.85,95.92},{76.59,132.38},
  {49.60,117.48},{54.00,114.01},{92.20,160.82},{98.73,180.85},
  {56.18,108.40},{75.10,144.54},{36.91,105.85},{36.52,75.99},
  {47.65,79.93},{72.52,151.63},{50.78,112.17},{85.76,151.27},
  {14.41,51.07},{48.66,109.44},{50.39,104.20},{95.63,174.36},
  {69.74,140.10},{25.53,87.75},{67.15,141.26},{55.44,129.77},
  {20.44,65.95},{98.96,181.37},{47.04,111.25},{58.45,117.28},
  {57.79,123.58},{83.84,171.31},{61.97,131.09},{37.66,109.35},
  {65.32,124.56},{32.31,92.17},{64.08,140.31},{53.53,119.28},
  {46.02,102.80},{ 8.19,46.99},{75.24,134.02},{11.54,67.14},
  {37.35,94.89},{17.90,73.66},{68.74,127.84},{35.61,74.80},
  {12.89,61.03},{51.57,127.06},{69.44,137.16},{55.00,87.07},
  { 9.46,66.57},{ 7.32,50.65},{18.69,60.81},{91.80,157.00},
  {23.13,80.31},{79.77,160.31},{25.92,67.57},{88.84,157.95},
  {59.62,122.91},{20.02,74.74},{66.18,130.20},{22.33,67.57},
  {34.98,72.13},{27.88,80.57},{74.11,152.93},{63.45,123.91},
  {34.17,76.27},{59.99,137.07},{26.78,70.38},{30.99,79.08},
  { 3.26,42.81},{51.85,97.62},{38.00,89.05},{69.57,128.69},
  {55.77,102.44},{38.64,95.04},{31.94,80.16},{33.57,84.21},
  {49.34,115.16},{60.37,126.82},{48.79,79.20},{53.62,101.98},
  {44.49,120.58},{40.72,92.32},{62.30,134.92},{86.71,167.45},
  {28.92,90.49},{ 3.64,50.38},{ 2.98,51.03},{ 7.72,50.72},
  {36.01,88.98},{12.80,53.33},{18.18,59.93},{ 8.74,39.72},
  {93.05,157.51},{78.51,157.36},{ 9.32,51.91},{28.43,83.25},
  { 5.63,57.81},{ 5.75,49.78},{33.39,89.81},{98.06,183.34},
  {61.33,124.19},{90.28,167.83},{97.93,180.90},{24.74,74.19},
  {28.42,90.02},{67.13,126.96},{29.53,65.54},{28.34,82.58},
  { 5.75,35.03},{36.57,89.60},{18.70,68.23},{85.61,147.33},
  {90.19,148.59},{29.44,82.07},{95.34,164.13},{25.79,76.89},
  {18.18,55.48},{47.36,99.63},{ 9.34,53.88},{95.16,174.51},
  {79.94,148.24},{ 1.82,30.86},{75.83,138.77},{15.10,60.29},
  {34.45,91.55},{ 0.41,39.02},{12.90,64.51},{75.66,161.10},
  {40.13,85.95},{57.37,109.43},{79.64,155.61},{25.54,76.82},
  {33.13,80.14},{31.55,75.12},{ 0.93,51.83},{36.40,97.82},
  {55.60,112.28},{ 1.26,47.44},{73.54,125.72},{49.81,102.51},
  {97.31,170.44},{89.64,169.02},{91.89,169.61},{45.89,107.66},
  {99.90,168.98},{39.27,106.49},{63.61,118.52},{60.86,115.28},
  { 3.11,48.70},{61.64,119.97},{28.18,81.53},{ 5.20,43.98},
  {51.76,117.25},{57.63,127.76},{78.84,153.80},{10.54,48.23},
  {52.51,115.61},{ 4.69,28.30},{93.93,164.59},{69.70,141.08},
  {57.13,118.40},{23.45,86.44},{44.59,132.03},{31.91,98.03},
  {44.44,103.85},{ 9.46,53.83},{92.57,190.48},{36.29,95.15},
  {32.06,92.48},{86.16,138.27},{49.86,112.94},{96.14,181.82},
  {16.05,59.03},{52.13,102.89},{51.27,109.44},{67.94,117.86},
  {86.77,158.44},{74.20,143.83},{39.13,93.09},{66.00,137.66},
  {22.48,55.93},{54.90,116.64},{51.71,95.82},{36.07,85.96},
  { 5.38,59.94},{84.49,160.75},{28.93,82.85},{89.92,183.08},
  { 1.83,61.19},{26.71,73.35},{49.96,93.82},{13.56,68.73},
  {26.93,70.29},{85.19,165.22},{74.31,148.26},{44.90,107.17},
  {81.60,159.66},{86.68,150.41},{ 8.77,66.43},{75.18,159.06},
  { 8.86,45.78},{66.61,131.48},{ 5.80,40.46},{84.73,169.37},
  {35.34,85.87},{97.62,161.97},{15.22,73.73},{77.52,152.32},
  { 2.96,45.12},{60.66,127.28},{66.50,131.20},{72.85,141.01},
  {65.90,130.75},{43.44,101.16},{11.06,52.18},{14.77,62.27},
  {77.52,159.43},{47.99,126.74},{63.43,110.36},{50.26,113.37},
  {95.43,198.57},{24.53,57.71},{12.87,54.28},{63.79,135.32},
  {56.58,110.85},{59.10,121.23},{99.99,194.48},{34.56,83.22},
  {98.83,173.25},{ 7.73,70.43},{34.44,103.18},{96.37,169.25},
  {50.19,118.06},{84.66,175.92},{79.01,142.91},{99.12,174.11},
  {61.97,120.41},{81.93,146.96},{36.18,71.82},{ 3.47,51.04},
  { 0.07,52.79},{82.02,158.42},{37.02,94.32},{26.83,77.25},
  {87.90,166.44},{22.65,76.05},{ 8.80,53.89},{53.12,109.05},
  {64.69,147.19},{55.56,107.54},{98.42,175.01},{ 4.99,30.66},
  {63.23,122.97},{56.44,121.49},{ 8.58,69.79},{79.38,149.69},
  {55.64,122.77},{84.76,142.77},{29.21,85.68},{18.11,70.14},
  {33.15,85.38},{11.30,44.68},{83.67,163.51},{23.43,101.91},
  {29.59,81.85},{19.90,75.18},{26.05,89.95},{61.05,129.50},
  { 6.27,68.93},{96.95,191.69},{82.78,145.43},{73.84,136.65},
  {60.44,140.08},{12.67,65.86},{ 2.20,36.68},{ 6.16,54.50},
  {35.28,95.48},{83.01,152.33},{64.33,121.41},{91.72,162.34},
  {72.62,130.77},{55.31,121.78},{83.52,164.02},{ 1.45,24.37},
  {84.96,164.70},{ 6.85,59.62},{89.95,158.91},{57.41,133.29},
  { 6.13,58.72},{78.33,125.38},{65.15,121.67},{19.10,81.33},
  {17.43,58.99},{60.92,142.74},{69.37,142.70},{ 7.71,53.52},
  {38.81,101.59},{18.31,65.35},{41.05,90.89},{29.84,96.28},
  {27.65,88.04},{11.74,30.21},{72.14,150.80},{59.92,123.34},
  {20.59,51.65},{73.54,147.09},{25.04,52.81},{21.44,78.92},
  { 8.30,68.18},{38.27,103.50},{76.73,135.46},{13.41,42.84},
  { 9.77,50.17},{31.79,84.64},{11.63,47.87},{81.10,154.34},
  {32.88,86.39},{83.66,156.16},{42.84,101.97},{92.23,181.69},
  {56.62,128.67},{21.57,72.57},{28.42,76.81},{78.49,151.43},
  {34.76,87.12},{95.65,153.86},{48.99,114.03},{22.97,82.24},
  {96.82,167.98},{55.42,93.17},{59.22,121.38},{41.66,101.95},
  {77.91,166.28},{12.14,54.69},{91.24,171.96},{46.22,106.58},
  {98.54,161.56},{46.98,104.41},{60.09,131.79},{67.83,119.99},
  {41.09,85.10},{70.10,135.73},{36.99,91.78},{10.72,53.99},
  {50.06,91.80},{18.39,60.80},{26.53,85.06},{76.65,154.26},
  { 0.02,34.66},{37.56,104.13},{91.48,177.82},{ 0.31,41.81},
  {93.20,166.21},{ 6.94,43.44},{85.38,159.15},{90.74,139.02},
  {83.80,157.57},{39.34,92.96},{95.49,170.28},{77.55,145.57},
  {33.05,88.61},{ 4.28,43.15},{95.89,183.02},{36.18,97.42},
  {94.88,160.54},{ 5.18,55.47},{24.04,66.69},{50.93,118.02},
  {11.82,58.02},{ 4.55,62.84},{71.87,136.43},{ 1.64,46.82},
  {86.64,157.08},{76.78,135.11},{17.94,59.70},{41.81,96.43},
  {89.21,170.11},{75.89,139.66},{90.73,160.27},{45.39,101.75},
  {61.07,135.71},{ 7.53,48.91},{26.32,74.50},{89.44,164.11},
  {79.85,162.30},{55.91,104.02},{89.47,172.39},{88.09,154.40},
  {87.63,148.99},{24.43,74.77},{49.28,103.08},{86.49,165.82},
  {93.79,148.29},{93.25,170.91},{59.55,126.98},{ 9.24,64.46},
  {73.82,134.35},{76.07,152.94},{77.44,148.98},{87.01,161.01},
  {72.52,154.33},{21.77,97.29},{47.74,111.47},{17.34,68.45},
  {39.75,119.04},{84.78,160.15},{62.57,121.76},{70.20,146.15},
  { 7.08,50.89},{60.33,97.44},{29.90,89.84},{41.82,95.48},
  {38.27,101.32},{ 9.60,53.87},{84.69,166.21},{97.64,177.02},
  {73.96,145.64},{11.68,73.29},{31.64,75.20},{44.12,119.54},
  {29.91,99.48},{62.85,117.28},{65.55,123.70},{78.66,161.94},
  {71.06,158.16},{71.17,147.50},{12.49,63.11},{62.47,146.21},
  { 5.52,64.77},{19.39,81.84},{90.73,177.46},{46.94,101.25},
  {35.30,92.84},{25.31,80.86},{29.29,95.30},{79.68,160.63},
  {65.64,143.96},{81.97,173.79},{58.68,123.92},{37.35,94.26},
  {81.31,146.56},{10.82,34.36},{72.19,152.74},{96.81,157.39},
  {37.82,84.01},{26.02,85.45},{49.68,99.80},{63.61,134.18},
  {78.45,135.62},{28.06,99.20},{29.49,76.17},{38.73,90.19},
  {66.67,128.82},{ 4.14,43.63},{75.01,155.68},{12.38,39.83},
  {64.06,126.87},{77.44,154.01},{30.67,89.38},{30.38,85.81},
  {98.17,180.45},{72.62,131.67},{18.80,74.37},{56.99,112.44},
  {80.45,152.36},{43.87,96.62},{10.95,79.63},{ 8.13,48.84},
  {96.47,180.58},{57.99,139.71},{29.81,82.98},{ 7.69,59.04},
  {60.75,113.11},{61.26,127.28},{29.91,74.20},{72.81,130.29},
  {97.22,186.22},{16.10,67.14},{45.22,82.91},{59.90,136.15},
  {50.86,99.99},{40.09,89.91},{38.69,87.12},{38.22,86.34},
  {82.85,160.54},{44.59,114.27},{ 6.39,49.22},{53.02,118.97},
  {67.10,132.43},{87.17,167.48},{61.46,109.49},{79.66,163.17},
  {40.28,88.74},{81.76,164.45},{10.26,58.64},{14.58,81.13},
  {85.30,184.26},{64.06,132.71},{ 5.55,56.52},{96.97,187.38},
  {92.22,174.95},{42.45,100.51},{30.79,81.24},{ 4.25,61.71},
  {47.15,104.16},{35.87,86.39},{81.62,152.64},{42.46,95.25},
  {66.69,137.47},{33.21,84.65},{23.42,84.12},{99.30,187.76},
  {19.15,77.26},{17.74,70.35},{87.90,170.12},{47.01,118.00},
  {78.63,155.19},{92.38,163.60},{72.75,153.70},{79.92,138.69},
  {21.94,78.76},{55.51,120.91},{27.08,57.31},{12.83,45.59},
  {48.22,103.52},{35.64,87.26},{59.90,119.91},{50.05,110.55},
  { 0.23,41.68},{66.03,129.51},{42.67,95.15},{37.78,103.08},
  { 3.06,43.68},{53.80,102.89},{ 9.78,51.90},{94.94,185.83},
  {31.69,105.92},{70.50,123.84},{ 5.52,51.03},{ 0.93,47.63},
  {68.12,146.17},{ 6.86,51.21},{ 4.60,42.38},{72.98,138.03},
  {58.59,125.26},{40.21,88.92},{12.51,41.25},{31.12,65.03},
  {75.68,143.15},{74.02,141.52},{ 5.61,50.98},{82.39,162.02},
  {28.07,65.38},{71.22,145.28},{44.22,99.25},{72.03,123.62},
  {45.88,95.49},{76.37,136.85},{29.19,81.03},{63.46,142.45},
  {49.44,100.25},{81.71,132.07},{83.34,150.05},{38.88,93.91},
  {86.01,172.44},{51.32,110.43},{86.82,154.64},{70.02,140.53},
  {26.43,72.25},{34.48,91.08},{30.41,80.09},{24.77,80.43},
  { 8.14,53.39},{18.88,70.38},{26.90,73.96},{94.43,173.51},
  {24.45,62.18},{56.07,111.25},{66.96,136.85},{93.78,188.93},
  {75.18,144.91},{18.22,43.66},{97.70,170.91},{34.25,95.34},
  {12.16,53.60},{88.48,164.67},{81.58,176.86},{81.96,151.45},
  {50.13,109.26},{44.20,90.81},{52.84,121.91},{17.30,76.64},
  {53.60,120.26},{32.01,84.79},{72.56,149.71},{19.15,55.05},
  {78.26,164.32},{ 9.84,56.96},{ 2.48,50.11},{50.84,134.33},
  {90.65,164.89},{35.58,81.09},{72.54,151.27},{54.39,119.73},
  {44.15,105.72},{74.88,145.20},{86.66,158.64},{17.79,67.78},
  {54.84,115.87},{99.10,173.71},{93.02,174.29},{23.52,83.88},
  {19.56,68.81},{24.03,79.83},{11.73,35.38},{ 3.82,37.84},
  {61.92,130.48},{77.02,139.38},{91.29,161.65},{98.09,162.25},
  { 0.49,36.07},{75.44,138.86},{ 4.32,59.64},{79.99,143.62},
  {13.43,47.42},{44.44,110.46},{25.03,71.30},{71.95,147.86},
  {78.51,152.59},{ 3.43,34.40},{55.28,115.97},{88.77,165.45},
  {15.43,82.65},{99.09,179.00},{79.77,143.93},{52.73,116.02},
  {52.40,109.06},{37.24,83.30},{31.90,80.93},{68.13,127.42},
  {70.63,127.66},{55.84,132.35},{39.95,99.29},{ 6.84,40.38},
  {66.47,117.89},{20.53,83.00},{82.22,147.22},{23.74,73.03},
  {77.83,159.15},{11.29,64.73},{49.15,104.01},{52.54,105.95},
  {93.36,160.71},{51.35,106.68},{28.56,83.86},{78.27,147.91},
  { 0.25,50.56},{59.93,120.88},{ 8.58,49.04},{74.24,134.96},
  {51.22,98.10},{24.91,74.31},{87.07,160.74},{52.25,105.22},
  {91.43,152.78},{ 8.10,59.46},{94.97,178.64},{88.81,178.24},
  {88.45,150.89},{21.60,76.09},{70.62,122.85},{99.65,168.36},
  {73.32,142.58},{13.18,71.21},{37.26,88.09},{79.15,142.60},
  {20.05,91.09},{33.64,87.49},{21.84,64.04},{49.12,116.82},
  {52.57,125.37},{42.43,93.45},{22.54,94.46},{82.51,165.14},
  {77.64,132.17},{32.25,83.61},{10.77,55.92},{71.34,133.82},
  {60.85,127.38},{22.68,79.72},{30.62,77.71},{81.90,161.50},
  {10.22,47.19},{26.58,57.16},{43.66,113.00},{90.69,145.82},
  {12.64,58.91},{85.90,154.22},{18.03,53.36},{84.49,144.57},
  {87.51,169.54},{92.50,170.96},{51.99,123.08},{45.16,108.57},
  {71.40,137.44},{58.36,121.84},{76.06,143.46},{42.17,104.52},
  { 2.57,50.10},{11.44,41.77},{71.09,143.92},{88.92,151.09},
  {92.79,177.42},{90.72,157.64},{66.11,141.52},{ 2.33,38.80},
  {76.26,158.72},{76.52,150.07},{70.31,132.13},{52.77,119.85},
  {99.59,176.94},{ 8.16,56.11},{99.29,190.79},{25.00,77.22},
  {13.45,63.42},{17.35,70.05},{ 4.16,35.31},{86.57,152.50},
  {88.57,168.14},{67.96,123.67},{72.36,142.41},{10.95,73.06},
  {78.45,163.31},{71.69,139.46},{82.78,157.91},{80.14,161.51},
  {60.33,133.76},{ 9.44,46.00},{68.21,163.48},{30.78,88.27},
  {38.74,105.24},{19.52,62.94},{49.03,105.82},{76.01,138.95},
  {71.08,165.10},{49.97,108.11},{75.15,145.38},{ 5.20,62.33},
  {97.13,188.35},{87.18,176.82},{42.70,96.24},{62.98,126.92},
  {96.09,175.08},{90.77,190.99},{71.68,124.23},{15.67,61.12},
  {95.37,178.30},{40.64,83.70},{22.64,71.16},{30.22,105.64},
  {18.96,77.17},{56.47,98.36},{36.53,84.99},{13.11,73.50},
  {32.04,80.23},{72.49,135.67},{54.33,126.59},{13.54,51.56},
  {14.77,57.56},{24.09,90.04},{32.43,86.80},{ 3.82,43.03},
  {81.10,163.58},{45.39,96.22},{57.29,115.98},{76.10,151.06},
  { 7.74,56.38},{48.95,108.35},{40.07,101.13},{81.91,144.88},
  {64.47,124.53},{70.83,129.18},{ 7.05,44.06},{36.46,86.68},
  {32.53,89.07},{32.88,77.92},{ 6.62,29.48},{28.87,76.01},
  {37.36,90.05},{72.25,136.07},{81.47,173.47},{ 4.20,47.41},
  {98.64,166.84},{46.61,109.47},{45.38,88.61},{95.41,169.40},
  {66.63,122.42},{98.96,176.41},{77.60,166.70},{39.53,93.96},
  {73.29,138.23},{87.99,159.87},{34.35,91.01},{33.30,78.44},
  {29.29,78.05},{89.99,153.84},{ 3.90,31.79},{ 2.74,28.45},
  {74.07,144.41},{59.60,135.80},{83.19,154.17},{33.14,71.48},
  {71.18,127.25},{59.10,126.89},{14.88,60.57},{46.36,122.43},
  {97.68,166.02},{47.91,110.00},{94.43,185.03},{25.13,73.98},
  {30.66,83.04},{47.36,100.33},{20.03,57.00},{38.53,77.34},
  {53.29,122.85},{77.72,146.52},{23.42,84.64},{96.85,170.99},
  { 8.49,68.27},{71.67,127.11},{84.22,158.75},{35.25,87.63},
  {74.00,140.25},{32.42,91.48},{88.91,156.95},{88.11,163.18},
  {60.49,132.55},{63.94,149.75},{95.21,172.49},{ 2.14,37.70},
  { 2.33,36.29},{24.38,83.62},{87.11,162.19},{37.16,85.65},
  {81.57,152.86},{49.26,116.82},{54.72,108.56},{65.82,132.02},
  {10.93,47.63},{71.92,111.28},{12.67,36.23},{67.35,101.56},
  {86.25,169.18},{97.89,194.43},{40.63,106.58},{73.87,135.71}
};
