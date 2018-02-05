#include "shm_rcv.h"

hmap_t* shm_get(){
  key_t key = ftok(SHM_FILE, 0x01);
  int shmid;
  if((shmid = shmget(key, sizeof(hmap_t)*SHM_SIZE, IPC_CREAT|0664)) < 0){
    log_fatal("shm_get %s failed",SHM_FILE);
    exit(EXIT_FAILURE);
  }
  char* shm;
  if((shm = shmat(shmid, NULL, SHM_RDONLY)) == (char*) -1){
    log_fatal("shmat %d failed", shmid);
    exit(EXIT_FAILURE);
  }
  hmap_t* pmap = (hmap_t*)shm;
  if(pmap){
    return pmap;
  }else{
    return NULL;
  }
}
