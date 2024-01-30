#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void panic(char*);

int
main(int argc, char* argv[])
{ 
  int fac = 2;
  int p[36][2];
  pipe(p[fac]);
  int pid = fork();
  if(pid == -1){
    panic("fork");
  }
  if(pid != 0){
    close(p[fac][0]);
    for(int i = 2; i <= 35; ++i){
      if(i <= fac){
        fprintf(1, "prime %d\n", i);
      } else if(i % fac != 0){
        write(p[fac][1], &i, 4);
      }
    }
    close(p[fac][1]);
    wait(0);
  } else {
    while(fac < 35){
      close(p[fac][1]);
      fac += 1;
      if(pipe(p[fac]) < 0){
        panic("pipe");
      }
      pid = fork();
      if(pid == -1){
        panic("fork");
      }
      if(pid != 0){
        int num;
        while(read(p[fac-1][0], &num, 4)){
          if(num == 0){
            break;
          }
          if(num <= fac){
            fprintf(1, "prime %d\n", num);
          } else if(num % fac != 0){
            write(p[fac][1], &num, 4);
          }
        }
        close(p[fac-1][0]);
        close(p[fac][1]);
        wait(0);
        exit(0);
      }
      close(p[fac-1][0]); // important
    }
  }
  exit(0);
}

void panic(char* s){
  fprintf(2, "%s\n", s);
  exit(0);
}