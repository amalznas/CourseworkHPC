#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <malloc.h>
/******************************************************************************

  Compile with:
    cc -o multithread0608 multithread0608.c -lcrypt -pthread

    ./multithread0608 > results.txt

******************************************************************************/
int n_passwords = 4;

char *encrypted_passwords[] = {
  "$6$KB$6SsUGf4Cq7/Oooym9WWQN3VKeo2lynKV9gXVyEG4HvYy1UFRx.XAye89TLp/OTcW7cGpf9UlU0F.cK/S9CfZn1",
  "$6$KB$1ocIiuN6StvEskjsYoYBid/gy8zXybieNCm9uM94nRw.ik9I04W3DJg0E52dswnozLmM0BIlzRZxgd.TleBwp1",
  "$6$KB$L4mWcpv6rMAbZdxfSsuAL2UZhbJ4vSGAAxk.vEcRKvIuPpwcSRKHzi3BXzWQWaH1p1ubwaFl.06CRQv6bVo3M1",
  "$6$KB$OpOIZac00sMBfYemANRTO3lKNZCFLegKAMakI3i2fk78/vZgo01X5mdG/1R1K0Ohs0V1AuxfOK7KY.th3dInb0"
};

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}

void crack(char *encrypted_passwords){
  pthread_t thread_1,thread_2;

  void *kernel_function_1();
  void *kernel_function_2();

  pthread_create(&thread_1, NULL, kernel_function_1, (void *)encrypted_passwords);
  pthread_create(&thread_2, NULL, kernel_function_2, (void *)encrypted_passwords);
 
  pthread_join(thread_1, NULL);
  pthread_join(thread_2, NULL);
}

void *kernel_function_1(char *salt_and_encrypted){
  int s, t, u;     
  char salt[7];    
  char plain[7];   
  char *enc;      
  int count = 0;   

  substr(salt, salt_and_encrypted, 0, 6);

  for(s='A'; s<='M'; s++){
    for(t='A'; t<='Z'; t++){
      for(u=0; u<=99; u++){
        sprintf(plain, "%c%c%02d", s, t, u); 
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
  printf("%d solutions explored\n", count);
}

void *kernel_function_2(char *salt_and_encrypted){
  int s, t, u;     
  char salt[7];    
  char plain[7];   
  char *enc;      
  int count = 0;   

  substr(salt, salt_and_encrypted, 0, 6);

  for(s='N'; s<='Z'; s++){
    for(t='A'; t<='Z'; t++){
      for(u=0; u<=99; u++){
        sprintf(plain, "%c%c%02d", s, t, u); 
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
  printf("%d solutions explored\n", count);
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


int main(int argc, char *argv[]){
  int i;
  
 struct timespec start, finish;   
  long long int time_elapsed;

  clock_gettime(CLOCK_MONOTONIC, &start);
  for(i=0;i<n_passwords;i<i++) {
    crack(encrypted_passwords[i]);
  }
 
  clock_gettime(CLOCK_MONOTONIC, &finish);
  time_difference(&start, &finish, &time_elapsed);
  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed,
                                         (time_elapsed/1.0e9)); 
  return 0;
}
