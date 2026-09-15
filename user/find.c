#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int matchhere(char *, char *);
int matchstar(int, char *, char *);

int
match(char *re, char *text)
{
    if(re[0] == '^')
        return matchhere(re + 1, text);

    do {
        if(matchhere(re, text))
            return 1;
    } while(*text++ != '\0');

    return 0;
}

int
matchhere(char *re, char *text)
{
    if(re[0] == '\0')
        return 1;

    if(re[1] == '*')
        return matchstar(re[0], re + 2, text);

    if(re[0] == '$' && re[1] == '\0')
        return *text == '\0';

    if(*text != '\0' && (re[0] == '.' || re[0] == *text))
        return matchhere(re + 1, text + 1);

    return 0;
}

int
matchstar(int c, char *re, char *text)
{
    do {
        if(matchhere(re, text))
            return 1;
    } while(*text != '\0' &&
            (*text++ == c || c == '.'));

    return 0;
}

void
runexec(char *path, char *cmdargs[], int cmdargc)
{
    int pid = fork();

    if(pid == 0){
        char *args[MAXARG];

        int i;
        for(i = 0; i < cmdargc; i++){
            args[i] = cmdargs[i];
        }

        args[i] = path;
        args[i + 1] = 0;

        exec(args[0], args);

        fprintf(2, "find: exec failed\n");
        exit(1);
    }

    wait(0);
}

void
find(char *path, char *target, int doexec, char *cmdargs[], int cmdargc)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    char *name = path + strlen(path);

    while(name > path && *(name - 1) != '/')
        name--;

    if(match(target, name)){
        if(doexec)
            runexec(path, cmdargs, cmdargc);
        else
            printf("%s\n", path);
    }

    if(st.type == T_DIR){
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
            fprintf(2, "find: path too long\n");
            close(fd);
            return;
        }

        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            if(strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
                continue;

            find(buf, target, doexec, cmdargs, cmdargc);
        }
    }

    close(fd);
}

int
main(int argc, char *argv[])
{
    if(argc < 3){
        fprintf(2, "Usage: find path filename [-exec command ...]\n");
        exit(1);
    }

    int doexec = 0;
    char **cmdargs = 0;
    int cmdargc = 0;

    if(argc > 3){
        if(strcmp(argv[3], "-exec") != 0){
            fprintf(2, "Usage: find path filename [-exec command ...]\n");
            exit(1);
        }

        if(argc < 5){
            fprintf(2, "find: no command after -exec\n");
            exit(1);
        }

        doexec = 1;
        cmdargs = &argv[4];
        cmdargc = argc - 4;
    }

    find(argv[1], argv[2], doexec, cmdargs, cmdargc);

    exit(0);
}
