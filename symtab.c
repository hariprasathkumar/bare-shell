
#include "symtab.h"
#include "print.h"
#include "string.h"
#include "syscall.h"
#include "memory.h"
#include "heap.h"
#include "map.h"

#define MEGA_BYTE   1024*1024

static void split_path(char *path)
{
    while (*path) {
        if (*path == ':') *path = '\0';
        path++;
    }
}

static long load_dir_into_symtab(struct hash **symtab, const char *dir)
{
    //my_printf("%s\n", dir);
    int fd = sys_openat(AT_FDCWD, dir, O_RDONLY | O_DIRECTORY, 0);
    if (fd < 0) {
        my_printf("error!\n");
        return -1;
    }

    char *buf = my_malloc(MEGA_BYTE);
    if (!buf) {
        my_printf("malloc failed for 1 Megabyte\n");
        return -1;
    }
    // hack to see allocated size
    // struct block *f = (struct block *)((uintptr_t)buf - sizeof(struct block));
    // my_printf("size allocated %d\n", f->size);
    //my_printf("current heap size %d\n", get_heap_size());
    char *full_path = my_malloc(512);
    if (!full_path) {
        my_printf("path buffer alloca failed\n");
        my_free(buf);
        return -1;
    }

    int bytesToRead = sys_getdents64(fd, (struct linux_dirent64 *)buf, MEGA_BYTE);
    if (bytesToRead < 0) 
    {
        my_free(buf);
        my_free(full_path);
        return -1;
    }

    struct linux_dirent64 *e = (struct linux_dirent64 *)buf;

    for (int entry = 0; entry < bytesToRead; entry += e->d_reclen, e = (struct linux_dirent64 *)((char *)e + e->d_reclen) )
    {
        char *filename = e->d_name;

        if ( (filename[0] == '.' && filename[1] == '\0') ||
             (filename[0] == '.' && filename[1] == '.' && filename[2] == '\0') ) continue;
        
        my_strncpy(full_path, dir, 512);
        size_t prefix = my_strlen(full_path);
        full_path[prefix] = '/';
        if ((prefix +  my_strlen(filename) + 1) >= 512) continue; // not fitting

        my_strncpy(&full_path[prefix+1], filename, 512 - (prefix + 1));

        if (sys_access(full_path, X_OK) == 0)
        {
            map_insert(symtab, filename, full_path, 0);
            //my_printf("d_name: %s d_type: %d full path: %s\n", e->d_name, e->d_type, full_path);
        }
    }

    sys_close(fd);

    my_free(buf);
    my_free(full_path);

    return 0;
}

long symtab_get_home_directory(char *home, size_t home_len, char **envp)
{
    my_memset(home, 0, home_len);

    while (*envp) {
        if (my_strncmp("HOME", *envp, 4) == 0) {
            size_t len = my_strlen(*envp+5);

            if (len <= home_len) {
                (void)my_memcpy(home, *envp+5, len);
                return 0;
            }
            return -1;
        }
        envp++;
    }
    return -1;
}

long symtab_build_from_env(struct hash **symtab, char **envp)
{
    char *path = NULL;

    while (*envp) {
        if (my_strncmp("PATH", *envp, 4) == 0)
        {
            //my_printf("%s\n", *envp);
            size_t len = my_strlen(*envp + 5);
            path = my_malloc(len+2);
            if (!path) {
                return -1;
            }

            (void)my_memcpy(path, *envp + 5, len);
            split_path(path);
            *(path + len) = '\0';
            *(path + len + 1) = '\0';

            char *p = path;
            while (*p) {
                if (load_dir_into_symtab(symtab, p) == -1)
                {
                    return -1;
                }
                p += my_strlen(p) + 1;
            }
            my_free(path);
        }
        envp++;
    }

    return 0;
}
