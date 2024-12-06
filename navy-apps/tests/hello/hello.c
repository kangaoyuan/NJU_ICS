#include <unistd.h>
#include <stdio.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  int cnt = 0;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 100000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
      if(++cnt == 7){
        break; 
      }
    }
  }
  return 0;
}
