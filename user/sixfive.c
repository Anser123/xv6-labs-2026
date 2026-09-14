#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void
process(int fd)
{
    char c;
    char num[32];
    int i = 0;

    while(read(fd, &c, 1) == 1){
        if(c >= '0' && c <= '9'){
            if(i < sizeof(num) - 1){
                num[i++] = c;
            }
        }
        else if(strchr(" -\r\t\n./,", c)){
            if(i > 0){
                num[i] = 0;

                int n = atoi(num);

                if(n % 5 == 0 || n % 6 == 0)
                    printf("%d\n", n);

                i = 0;
            }
        }
        else{
            // Not a separator, so this is not a valid standalone number.
            i = 0;
        }
    }

    // EOF also acts like a separator.
    if(i > 0){
        num[i] = 0;

        int n = atoi(num);

        if(n % 5 == 0 || n % 6 == 0)
            printf("%d\n", n);
    }
}

int
main(int argc, char *argv[])
{
    if(argc == 1){
        process(0);
        exit(0);
    }

    for(int i = 1; i < argc; i++){
        int fd = open(argv[i], O_RDONLY);

        if(fd < 0){
            printf("sixfive: cannot open %s\n", argv[i]);
            continue;
        }

        process(fd);
        close(fd);
    }

    exit(0);
}
