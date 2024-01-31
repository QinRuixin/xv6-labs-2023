#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char* argv[]){
  char buf[32];
  char* nargv[MAXARG+1];
  int len = 0;

  if(argc < 2){
    fprintf(2, "Usage: xargs commands...\n");
    exit(1);
  }
  
  for(int i = 0; i < argc-1; ++i){
    nargv[i] = argv[i+1];
  }

  while (1)
  {
    int n;
    while((n = read(0, buf+len, 1)) > 0){
      len += n;
      if(buf[len-1]=='\n' || buf[len-1] == 0){
        break;
      }
    }
    
    int pid = fork();
    if( pid == 0){
      buf[len-1] = 0;
      nargv[argc-1] = buf;
      exec(argv[1], nargv);
      fprintf(2, "xargs: exec %s failed\n", argv[1]);
      exit(1);
    }

    if(buf[len-1] == 0){
      break;
    }
    for(int i = 0; i < len; ++i){
      buf[i] = 0;
    }
    len = 0;
  }
  wait(0);
  exit(0);
}
