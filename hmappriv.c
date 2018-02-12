#include "hmappriv.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "hashmap.h"
#include "memcached_client.h"

bool hmap_check(hmap_t* pmap, const char* container,const char* pname,const char* abs_path){
   char key[KEY_SIZE];
   sprintf(key,"%s:%s:ALLOW_LIST", container, pname);
   char* allow_data = (char*)get_item_hmap(pmap, key);
   log_debug("allow_data: %s",allow_data);
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

bool hmap_priv_check(hmap_t* pmap, const char* abs_path){
    //local check
    struct ProcessInfo pinfo;
    get_process_info(&pinfo);
    return hmap_check(pmap,"container",pinfo.pname,abs_path);
}

bool mem_check(const char* container, const char* pname, const char* abs_path){
   char allow_key[MEMCACHED_MAX_EKY];
   sprintf(allow_key,"%s:%s:allow",container,pname);
   char* value = getValue(allow_key);
   if(value){
       char* valret = NULL;
       char* rest = value;
       char* token = NULL;

       while((token = strtok_r(rest,";",&rest))){
           if((valret = strstr(abs_path, token))){
               return true;
           }else{
               return false;
           }
       }
   }
   return false;
}

bool mem_priv_check(const char* abs_path){
    struct ProcessInfo pinfo;
    get_process_info(&pinfo);
    return mem_check("container",pinfo.pname,abs_path);
}
