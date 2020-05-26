#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define SIZE 1024





void catcher(int sig) {
    errno = EINTR;
}


int main(int argc, char **argv) {
    if (argc != 2) {
        perror("wrong param");
        return -3;
    }
    int file_desc;
    if ((file_desc = open(argv[1], O_RDONLY)) == -1) {
        perror("can't open file");
        return -2;

    }
    char c;
    int str_pos = 0;
    int str = 0;
    int str_len[SIZE];
    int offset[SIZE];
    char buf[SIZE];

    while (read(file_desc, &c, 1)) {
        if (c == '\n') {
            ++str_pos;
            str_len[str] = str_pos;
            ++str;
            offset[str] = lseek(file_desc, 0, SEEK_CUR);
            str_pos = 0;

        } else ++str_pos;


    }

    int number;
    int i = 0;
    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    /*
     *
sigemptyset simply
     initializes the signalmask to empty,
     such that it is guaranteed that no signal would be masked.
     (that is, all signals will be received)
     Basically it is similar to a memset(0)
     but you don't have to know the details of the
     implementation. So if the sa_mask member is changed
     you don't need to adjust your code because it will be taken care of by
     sigemptyset.
     */
    sact.sa_flags = 0;
    sact.sa_handler = catcher;
    sigaction(SIGALRM, &sact, NULL);


    while (1) {
        printf("write number of string\n");
        alarm(5); /* заказать сигнал через 5 сек */
        read(STDIN_FILENO, &buf, SIZE);
        alarm(0);
        if (errno == EINTR) {
            lseek(file_desc, SEEK_SET, 0);

            while ((i = read(file_desc, buf, SIZE)) > 0)
                write(1, buf, i);
            exit(0);
        }


            number = atoi(buf);
            if (number == 0) {
                break;
            }
            if (number < 0) {
                printf("wrong format of number: ");
                return -1;
            }
            lseek(file_desc, offset[number], SEEK_SET);
            if (read(file_desc, buf, str_len[number])) {
                write(STDOUT_FILENO, buf, str_len[number]);
            } else {
                printf("bad number: ");

            }


        }


        return 0;
    }
