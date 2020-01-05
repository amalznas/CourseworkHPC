#include <stdio.h>
#include <cuda_runtime_api.h>
#include <time.h>
/****************************************************************************

  Compile with:
    nvcc -o pc_cuda pc_cuda.cu 
  Run with:
    ./pc_cuda

*****************************************************************************/

__device__ int is_a_match(char *attempt) {
	char plain_password1[] = "CV7828";
	char plain_password2[] = "FR8356";
	char plain_password3[] = "GS0484";
	char plain_password4[] = "TB4535";


	char *m = attempt;
	char *n = attempt;
	char *o = attempt;
	char *p = attempt;
	char *p1 = plain_password1;
	char *p2 = plain_password2;
	char *p3 = plain_password3;
	char *p4 = plain_password4;

	while(*m == *p1) { 
		if(*m == '\0') 
		{
			printf("Password: %s\n",plain_password1);
			break;
		}

		m++;
		p1++;
	}
	
	while(*n == *p2) { 
		if(*n == '\0') 
		{
			printf("Password: %s\n",plain_password2);
			break;
		}

		n++;
		p2++;
	}

	while(*o == *p3) { 
		if(*o == '\0') 
		{
			printf("Password: %s\n",plain_password3);
			break;
		}

		o++;
		p3++;
	}

	while(*p == *p4) { 
		if(*p == '\0') 
		{
			printf("Password: %s\n",plain_password4);
			return 1;
		}

		p++;
		p4++;
	}
	return 0;

}

__global__ void  kernel() {
	char i1,i2,i3,i4;

	char password[7];
	password[6] = '\0';

	int i = blockIdx.x+65;
	int j = threadIdx.x+65;
	char initialMatch = i; 
	char finalMatch = j; 

	password[0] = initialMatch;
	password[1] = finalMatch;
	for(i1='0'; i1<='9'; i1++){
		for(i2='0'; i2<='9'; i2++){
			for(i3='0'; i3<='9'; i3++){
				for(i4='0'; i4<='9'; i4++){
					password[2] = i1;
					password[3] = i2;
					password[4] = i3;
					password[5] = i4; 
					if(is_a_match(password)) {
					} 
					else {
	     			//printf("tried: %s\n", password);		  
					}
				}
			}
		}
	}
}

int time_difference(struct timespec *start, 
	struct timespec *finish, 
	long long int *difference) {
	long long int ds =  finish->tv_sec - start->tv_sec; 
	long long int dn =  finish->tv_nsec - start->tv_nsec; 
	if(dn < 0 ) {
		ds--;
		dn += 1000000000; 
	} 
	*difference = ds * 1000000000 + dn;
	return !(*difference > 0);
}

int main() {

	struct  timespec start, finish;
	long long int time_elapsed;
	clock_gettime(CLOCK_MONOTONIC, &start);

	kernel <<<26,26>>>();
	cudaThreadSynchronize();

	clock_gettime(CLOCK_MONOTONIC, &finish);
	time_difference(&start, &finish, &time_elapsed);
	printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed, (time_elapsed/1.0e9)); 

	return 0;
}


