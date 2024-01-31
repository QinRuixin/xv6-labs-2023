#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char buf[512];

void recfind(char* p, int fd, char* file);

int
main(int argc, char* argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: find directory files...\n");
    exit(1);
  }

  int fd;
  struct stat st;
  char* path = argv[1];

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }

  if(st.type != T_DIR){
    fprintf(2, "find: %s is not a directory\n", path);
    close(fd);
    exit(1);
  }

  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
    printf("find: path too long\n");
    close(fd);
    exit(1);
  }
  strcpy(buf, path);
  char *p = buf + strlen(buf);
  *p++ = '/';

  recfind(p, fd, argv[2]);

  close(fd);
  exit(0);
}

void recfind(char* p, int fd, char* filename){
  struct dirent de;
  struct stat st;
  while((read(fd, &de, sizeof(de))) == sizeof(de)){
    if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0){
      continue;
    }
    memmove(p, de.name, sizeof(de.name));
    p[sizeof(de.name)] = 0; // todo weired
    if(stat(buf, &st) < 0){
      printf("ls: cannot stat %s\n", buf);
      for(int i = 0; i < strlen(de.name); ++i){
        p[i] = 0;
      }
      continue;
    }
    if(st.type == T_DIR){
      int recfd;
      if((recfd = open(buf, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", buf);
        exit(1);
      }
      char *newp = buf + strlen(buf); // newp not p so no bug
      *newp++ = '/';
      recfind(newp, recfd, filename);
      close(recfd);
    } else if(strcmp(de.name, filename) == 0){
      printf("%s\n", buf);
    }
    for(int i = 0; i < strlen(de.name); ++i){
      p[i] = 0;
    }
    
  }
}