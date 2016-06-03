#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef int (*orig_access_f_type)(const char *pathname, int mode);
orig_access_f_type orig_access;

typedef int (*orig_open_f_type)(const char *pathname, int flags);
orig_open_f_type orig_open;

typedef int (*orig_stat_f_type)(const char *pathname, struct stat *buf);
orig_stat_f_type orig_stat;

typedef int (*orig_lstat_f_type)(const char *pathname, struct stat *buf);
orig_lstat_f_type orig_lstat;

char * strrep(char *str, char *o_s, char *n_s)
{
    char *newstr = NULL;
    char *c = NULL;

    /* no substring found */
    if ((c = strstr(str, o_s)) == NULL) {
        return str;
    }

    if ((newstr = (char *) malloc((int) sizeof(str) -
                                  (int) sizeof(o_s) +
                                  (int) sizeof(n_s) + 1)) == NULL) {
        printf("ERROR: unable to allocate memory\n");
        return NULL;
    }

    strncpy(newstr, str, c-str);
    sprintf(newstr+(c-str), "%s%s", n_s, c+strlen(o_s));

    return newstr;
}

int access(const char *pathname, int mode)
{
    struct stat sb;

    if (-1 == orig_lstat("/proc/self/exe", &sb)) {
       perror("lstat");
       exit(EXIT_FAILURE);
    }
    printf("Allocating %i bytes\n", (int)sb.st_size + 1);
    char *linkname = malloc(sb.st_size + 1);
    if (NULL == linkname) {
        fprintf(stderr, "insufficient memory\n");
        exit(EXIT_FAILURE);
    }
    ssize_t r = readlink("/proc/self/exe", linkname, sb.st_size);
    if (r == -1) {
       perror("readlink");
       exit(EXIT_FAILURE);
    }
    if (r > sb.st_size) {
       fprintf(stderr, "symlink increased in size "
                       "between lstat() and readlink()\n");
       exit(EXIT_FAILURE);
    }
    linkname[r] = '\0';

    int ret = orig_access(pathname, mode);
    if (0 == ret) {
        printf("Access: Found %s\n", pathname);
        return ret;
    }
    printf("Access: Not found: %s\n", pathname);
    char *newpath = (char*)pathname;

    printf("Accessing %s with mode %i\n", pathname, mode);
    if (NULL != strstr(pathname, ".so")) {
	printf("Relocating library\n");
        newpath = strrep(newpath, "Binaries", "Libraries");
        printf("Now accessing %s\n", newpath);
    }
    if (NULL != strstr(pathname, "/usr/share")) {
	printf("Relocating shared resource\n");
        newpath = strrep(newpath, "/usr/share", linkname);
        printf("Now accessing %s\n", newpath);
    }

    free(linkname);
    return orig_access(newpath, mode);
}

int open(const char *pathname, int flags)
{
    char *newpath = (char*)pathname;
    printf("Opening %s\n", pathname);
    if (NULL != strstr(pathname, ".so")) {
	printf("Relocating library\n");
        newpath = strrep(newpath, "Binaries", "Libraries");
        printf("Now opening %s\n", newpath);
    }
    return orig_open(newpath, flags);
}

int stat(const char *pathname, struct stat *buf)
{
    char *newpath = (char *)pathname;
    printf("Stating %s\n", pathname);
    if (NULL != strstr(pathname, ".so")) {
        printf("Relocating library\n");
        newpath = strrep(newpath, "Binaries", "Libraries");
        printf("Now opening %s\n", newpath);
    }
    return orig_stat(newpath, buf);
}

int lstat(const char *pathname, struct stat *buf)
{
    char *newpath = (char *)pathname;
    printf("Stating %s\n", pathname);
    if (NULL != strstr(pathname, ".so")) {
        printf("Relocating library\n");
        newpath = strrep(newpath, "Binaries", "Libraries");
        printf("Now opening %s\n", newpath);
    }
    return orig_lstat(newpath, buf);
}

void _init(void)
{
  orig_access = dlsym(RTLD_NEXT, "access");
  orig_open = dlsym(RTLD_NEXT, "open");
  orig_stat = dlsym(RTLD_NEXT, "stat");
  orig_lstat = dlsym(RTLD_NEXT, "lstat");
}
