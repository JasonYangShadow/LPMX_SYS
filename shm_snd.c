#include "shm_snd.h"

void shm_snd(hmap_t* pmap){
  key_t key = ftok(SHM_FILE, 0x01);
  int shmid;
  if((shmid = shmget(key, sizeof(hmap_t)*SHM_SIZE, IPC_CREAT|0664)) < 0){
    log_fatal("shm_get %s failed",SHM_FILE);
    exit(EXIT_FAILURE);
  }
  char* shm;
  if((shm = shmat(shmid,NULL,0)) == (char*)-1){
    log_fatal("shmat %d failed", shmid);
    exit(EXIT_FAILURE);
  }
  memcpy(shm,pmap,sizeof(hmap_t)*SHM_SIZE);
}
