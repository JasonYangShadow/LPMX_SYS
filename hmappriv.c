#include "hmappriv.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "hashmap.h"

bool hmap_check(hmap_t* pmap, const char* container,const char* pname,const char* abs_path){
   char* allow_data = (char*)get_complex_hmap(pmap,container, pname, "ALLOW_LIST");
   if(allow_data){
      char* valret = NULL;
      char* rest = allow_data;
      char* token = NULL;

      while((token = strtok_r(rest, ";", &rest))){
          if((valret = strstr(abs_path, token))){
             return true;
          }
      }
      return false;
   }
   return false;
}


void get_process_info(struct ProcessInfo* pinfo){
  char pathbase[PNAME_SIZE];
  FILE *fp;
  pinfo->pid = getpid();
  pinfo->ppid = getppid();
  pinfo->groupid = getpgrp();
  strcpy(pathbase,"/proc/");
  sprintf(pathbase+strlen(pathbase),"%d",pinfo->pid);
  sprintf(pathbase+strlen(pathbase),"%s","/status");
    
  fp = fopen(pathbase,"r");
  if(fp == NULL){
     log_fatal("can't open /proc/%d/status",pinfo->pid);
  }
  fscanf(fp,"Name:%s",pinfo->pname);
  fclose(fp);
}

bool privilege_check(hmap_t* pmap, const char* abs_path){
    //local check
    struct ProcessInfo pinfo;
    get_process_info(&pinfo);
    return hmap_check(pmap,"container",pinfo.pname,abs_path);
}
