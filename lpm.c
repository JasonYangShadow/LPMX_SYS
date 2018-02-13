/****************************************************************************************
 * lpm.c monitors the system calls of file/folder operations and logs into disk file
 *
 * This file is part of LPM project
 * Copyright (C) 2017, Kasahara Lab
 * For more information on LPM project, please visit https://lpm.bio
 * This project is shared and hosted on github, https://jasonyangshadow.github.io/LPM_LIB/
 *
****************************************************************************************/
#include "lpm.h"
#include "log.h"
#include "hmappriv.h"

static void lpm_log_rename(const char* oldpath, const char* newpath){
	char oldbuf[PATH_BUF_SIZE], newbuf[PATH_BUF_SIZE];
	struct stat st;
	DIR* dir;
	struct dirent* e;
	size_t oldlen, newlen;
	int old_errno = errno;

	/* The newpath file doesn't exist */
	if (lstat(newpath, &st) < 0) 
		goto goto_end;

	else if (!S_ISDIR(st.st_mode)) {
		log_debug("path old:%s => new: %s",oldpath,newpath);
		goto goto_end;
	}

	/* Make sure we have enough space for the following slashes */
	oldlen = strlen(oldpath);
	newlen = strlen(newpath);
	if (oldlen + 3 > PATH_BUF_SIZE || newlen + 3 > PATH_BUF_SIZE)
		goto goto_end;

	strcpy(oldbuf, oldpath);
	strcpy(newbuf, newpath);
	oldbuf[oldlen] = newbuf[newlen] = '/';
	oldbuf[++oldlen] = newbuf[++newlen] = 0;

	if (!(dir = opendir(newbuf)))
		goto goto_end;

	while ((e = readdir(dir))) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		strncat(oldbuf, e->d_name, PATH_BUF_SIZE- oldlen - 1);
		strncat(newbuf, e->d_name, PATH_BUF_SIZE- newlen - 1);
		lpm_log_rename(oldbuf, newbuf);
		oldbuf[oldlen] = newbuf[newlen] = 0;
	}

	closedir(dir);

goto_end: 
	errno = old_errno;
}



static void lpm_abs_path(int fd, const char* path, char* abs_path){
	char cwd[PATH_BUF_SIZE], aux[PATH_BUF_SIZE];
	int old_errno = errno;

	/* already absolute (or can't get CWD) */
	if (path[0] == '/' || !getcwd(cwd, PATH_BUF_SIZE)){
		strncpy(abs_path, path, PATH_BUF_SIZE - 1);
    }
	else if (fd < 0) {
		strncpy(abs_path, cwd, PATH_BUF_SIZE - 1);
		strncat(abs_path, "/", PATH_BUF_SIZE - strlen(abs_path) - 1);
		strncat(abs_path, path, PATH_BUF_SIZE - strlen(abs_path) - 1);
	}
	else if (fchdir(fd) == 0 && getcwd(aux, PATH_BUF_SIZE) && chdir(cwd) == 0) {
		strncpy(abs_path, aux, PATH_BUF_SIZE - 1);
		strncat(abs_path, "/", PATH_BUF_SIZE - strlen(abs_path) - 1);
		strncat(abs_path, path, PATH_BUF_SIZE - strlen(abs_path) - 1);
	}
	else{
		strncpy(abs_path, path, PATH_BUF_SIZE - 1);
    }
	abs_path[PATH_BUF_SIZE - 1] = 0;
	errno = old_errno;
}


static void* lpm_dlsym(const char *symbol){
    void* ret;
    if(!(ret=dlsym(RTLD_NEXT,symbol))){
        log_fatal("dlsym error:%s",(char*)dlerror());
        exit(EXIT_FAILURE);
    }
    return ret;
}

static void lpm_init(){
    //define the system hooks
    FUNC_DLSYM (open) 
    FUNC_DLSYM (creat) 
    FUNC_DLSYM (rename) 
    FUNC_DLSYM (link) 
    FUNC_DLSYM (symlink) 
    FUNC_DLSYM (fopen) 
    FUNC_DLSYM (freopen) 
    FUNC_DLSYM (unlink) 
    FUNC_DLSYM (rmdir) 
    FUNC_DLSYM (mkdir) 

    FUNC_DLSYM (open64) 
    FUNC_DLSYM (creat64) 
    FUNC_DLSYM (fopen64) 
    FUNC_DLSYM (freopen64) 

    FUNC_DLSYM (openat) 
    FUNC_DLSYM (renameat) 
    FUNC_DLSYM (linkat) 
    FUNC_DLSYM (symlinkat) 
    FUNC_DLSYM (unlinkat) 
    FUNC_DLSYM (mkdirat) 

    FUNC_DLSYM (openat64)
}

/**System Hooks implementation*/
int open(const char* path, int flags, ...){
	va_list a;
	int mode, accmode, ret;

	if (!strncmp(path, "/proc/", 6))
		return __open(path, flags);

	lpm_init();
	
  char abs_path[PATH_BUF_SIZE];
  lpm_abs_path(-1,path,abs_path);
  if(!mem_priv_check(abs_path)){
      log_fatal("[DENIED]open %s is not allowed",abs_path);
      exit(EXIT_FAILURE);
  }
	
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	if ((ret = libc_open(path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR)
			log_debug("syscall open: %s",path);
	}

	return ret;
}


int creat(const char* path, mode_t mode){
	int ret;
	
	lpm_init();
	
	if ((ret = libc_creat(path, mode)) != -1)
		log_debug("syscall create: %s ,  mode: %o",path,mode);
	
	return ret;
}


int rename(const char* oldpath, const char* newpath){
	int ret;
	
	lpm_init();
	
	if ((ret = libc_rename(oldpath, newpath)) != -1){
    lpm_log_rename(oldpath, newpath);
  }
	return ret;
}


int link(const char* oldpath, const char* newpath){
	int ret;
	
	lpm_init();
	
	if ((ret = libc_link(oldpath, newpath)) != -1){
		log_debug("syscall link old: %s => new: %s",oldpath, newpath);
  }	
	return ret;
}


int symlink(const char* oldpath, const char* newpath){
	int ret;
	
	lpm_init();
	
	if ((ret = libc_symlink(oldpath, newpath)) != -1){
		log_debug("syscall symlink old: %s => new: %s",oldpath, newpath);
  }
	
	return ret;
}


FILE* fopen(const char* path, const char* mode){
	FILE* ret;
	
	lpm_init();
	
	if ((ret = libc_fopen(path, mode)) && strpbrk(mode, "wa+")){
		log_debug("syscall fopen: %s,mode: %s",path,mode);
  }	
	return ret;
}


FILE* freopen(const char* path, const char* mode, FILE* stream){
	FILE* ret;
	
	lpm_init();
	
	if ((ret = libc_freopen(path, mode, stream)) && strpbrk(mode, "wa+")){
		log_debug("syscall freopen: %s,mode: %s",path,mode);
  }	
	return ret;
}

int unlink(const char* path){
    int ret;
    
    lpm_init();

    if((ret = libc_unlink(path))!=-1){
	    log_debug("syscall unlink: %s",path);
    }
    return ret;
}

int rmdir(const char* path){
    int ret;
    
    lpm_init();

    if((ret = libc_rmdir(path))!=-1){
	    log_debug("syscall rmdir: %s",path);
    }
    return ret;

}

int mkdir(const char* path, mode_t mode){
    int ret;

    lpm_init();

    if((ret = libc_mkdir(path,mode))!=-1){
        log_debug("syscall mkdir: %s,mode: %o",path,mode);
    }
    return ret;
}

int open64(const char* path, int flags, ...){
	va_list a;
	int mode, accmode, ret;
	
	if (path && !strncmp(path, "/proc/", 6))
		return __open64(path, flags);

	lpm_init();
    
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	
	if ((ret = libc_open64(path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR)
			log_debug("syscall open64: %s",path);
	}

	return ret;
}


int creat64(const char* path, mode_t mode){
	int ret;
	
	lpm_init();
	
	if ((ret = libc_creat64(path, mode)) != -1)
		log_debug("syscall create64: %s,mode: %o",path,mode);
	
	return ret;
}


FILE* fopen64(const char* path, const char* mode){
	FILE* ret;
	
	lpm_init();
	
	ret = libc_fopen64(path, mode);
	if (ret && strpbrk(mode, "wa+"))
		log_debug("syscall fopen64: %s,mode: %s",path,mode);
	
	return ret;
}


FILE* freopen64(const char* path, const char* mode, FILE* stream){
	FILE* ret;
	
	lpm_init();
	
	ret = libc_freopen64(path, mode, stream);
	if (ret && strpbrk(mode, "wa+"))
		log_debug("syscall freopen64: %s, mode: %s",path,mode);
	
	return ret;
}

int openat(int fd, const char* path, int flags, ...){
	va_list a;
	int mode, accmode, ret;
	char abs_path[PATH_BUF_SIZE];

	lpm_init();
	
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	
	if ((ret = libc_openat(fd, path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR) {
			lpm_abs_path(fd, path, abs_path);
			log_debug("syscall openat: %s,fd: %d", abs_path,fd);
		}
	}

	return ret;
}


int renameat(int oldfd, const char* oldpath, int newfd, const char* newpath){
	int ret;
	char old_abs_path[PATH_BUF_SIZE];
	char new_abs_path[PATH_BUF_SIZE];
	
	lpm_init();

	if ((ret = libc_renameat(oldfd, oldpath, newfd, newpath)) != -1) {
		lpm_abs_path(oldfd, oldpath, old_abs_path);
		lpm_abs_path(newfd, newpath, new_abs_path);
		lpm_log_rename(old_abs_path, new_abs_path);
	}

	return ret;
}


int linkat(int oldfd, const char* oldpath, 
           int newfd, const char* newpath, int flags){
	int ret;
	char new_abs_path[PATH_BUF_SIZE];
	
	lpm_init();

	if ((ret = libc_linkat(oldfd, oldpath, newfd, newpath, flags)) != -1) {
		lpm_abs_path(newfd, newpath, new_abs_path);
		log_debug("syscall linkat old:%s => new: %s", oldpath, new_abs_path);
	}

	return ret;
}


int symlinkat(const char* oldpath, int newfd, const char* newpath){
	int ret;
	char new_abs_path[PATH_BUF_SIZE];
	
	lpm_init();
	
	if ((ret = libc_symlinkat(oldpath, newfd, newpath)) != -1) {
		lpm_abs_path(newfd, newpath, new_abs_path);
		log_debug("syscall symlinkat old:%s => new: %s",oldpath,new_abs_path);
	}

	return ret;
}

int unlinkat(int fd, const char* path, int flags){
    char abs_path[PATH_BUF_SIZE];
    int ret;

    lpm_init();

    if((ret = libc_unlinkat(fd,path,flags))!=-1){
        lpm_abs_path(fd,path,abs_path);
        log_debug("syscall unlinkat: %s",abs_path);
    }
    return ret;
}

int mkdirat(int fd,const char* path,mode_t mode){
    char abs_path[PATH_BUF_SIZE];
    int ret;

    lpm_init();

    if((ret = libc_mkdirat(fd,path,mode))!=-1){
        lpm_abs_path(fd,path,abs_path);
        log_debug("syscall mkdirat: %s, mode: %o",abs_path,fd);
    }

    return ret;
}

int openat64(int fd, const char* path, int flags, ...){
	va_list a;
	int mode, accmode, ret;
	char abs_path[PATH_BUF_SIZE];

	lpm_init();
	
	va_start(a, flags);
	mode = va_arg(a, int);
	va_end(a);
	
	if ((ret = libc_openat64(fd, path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR) {
			lpm_abs_path(fd, path, abs_path);
			log_debug("syscall openat64: %s",abs_path);
		}
	}

	return ret;
}
