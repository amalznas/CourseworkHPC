#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>

/******************************************************************************

  Compile with:
    cc -o CrackAZ99-With-Data0608 CrackAZ99-With-Data0608.c -lcrypt

    ./CrackAZ99-With-Data0608 > results.txt

******************************************************************************/
int n_passwords = 4;

char *encrypted_passwords[] = {
  "$6$KB$t7URKN07IpCV3VKvOB827yLWDjxPsdVJJI9V5yqRbuEmepRDqjGAlapr2G4s6gKTCi82Hc1/NW/JR7v53WyXj/",
  "$6$KB$.KbTGUkIIbposunUqyRC56Lo7/XZohBNnyhkqQ1E4Xo.Cf6D0qK7/wMEdbWj7AOohuExF3tDqQA5gGt9MTuT30",
  "$6$KB$DOBoXJMpjGBLxcJePibVbPUiC2GdZgKinYenlPygdimi.TQ5dr.ZTgD01xLHXT6BT7beWXmFL6b/R8FxO/f4z.",
  "$6$KB$Og0bS2h6pgM57ZDNDibmpUtSPyHKSZLOp2wq.Bxv1grRloWJ0qNBeeoM3on5xdAtYjTHq3CgYpbfCcLb9.9Qp."
};

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}

void crack(char *salt_and_encrypted){
  int s, t, u, v;  
  char salt[7];    
  char plain[7];   
  char *enc;      
  int count = 0;   

  substr(salt, salt_and_encrypted, 0, 6);

  for(s='A'; s<='Z'; s++){
    for(t='A'; t<='Z'; t++){
	for(u='A'; u<='Z'; u++){
      		for(v=0; v<=99; v++){
        		sprintf(plain, "%c%c%c%02d", s, t, u, v); 
        		enc = (char *) crypt(plain, salt);
       			count++;
        		if(strcmp(salt_and_encrypted, enc) == 0){
          		printf("#%-8d%s %s\n", count, plain, enc);
        		} else {
          		printf(" %-8d%s %s\n", count, plain, enc);
        		}
      		}
	}
     }
  }
  printf("%d solutions explored\n", count);
}

int time_difference(struct timespec *start, struct timespec *finish, 
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

int main(int argc, char *argv[]) {
  int i;
  struct timespec start, finish;   
  long long int time_elapsed;

  clock_gettime(CLOCK_MONOTONIC, &start);

  for(i=0;i<n_passwords;i<i++){
     crack(encrypted_passwords[i]);
  }

  clock_gettime(CLOCK_MONOTONIC, &finish);
  time_difference(&start, &finish, &time_elapsed);
  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed, 
         (time_elapsed/1.0e9)); 

  return 0;
}
