#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void panic(char*);

int
main(int argc, char* argv[])
{
  int p[2];
  if(pipe(p) < 0){
    panic("pipe");
  }
  int pid = fork();
  if(pid == -1){
    panic("fork");
  }
  if(pid == 0){
    int ppid;
    pid = getpid();
    // fprintf(1, "%d: start son\n", pid);
    read(p[0], &ppid, 1);
    fprintf(1, "%d: received ping\n", ppid);
    write(p[1], &pid, 1);
    close(p[0]);
    close(p[1]);
  } else {
    int spid;
    pid = getpid();
    // fprintf(1, "%d: start parent\n", pid);
    write(p[1], &pid, 1);
    wait(0); // important
    read(p[0], &spid, 1);
    fprintf(1, "%d: received pong\n", spid);
    close(p[0]);
    close(p[1]);
  }
  exit(0);
}

void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}
