#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define SIZE 1024


int get_file_info(int file_desc, int *amount_of_lines, int **str_len,int **offset){
    *amount_of_lines = 0;
    int max_size = SIZE;
    (*str_len) = (int*) malloc(sizeof(int) * SIZE);
    if((*str_len) == NULL){
        perror("Failed to allocate memory for str_len in get_file_info\n");
        return -1;
    }
    (*offset) = (int*) malloc(sizeof(int) * SIZE);
    if((*offset) == NULL){
        perror("Failed to allocate memory for offset in get_file_info\n");
        return -1;
    }
    char c;
    int pos_in_str = 0;
    int str_num = 0;
    (*offset)[0] = 0;
    while (read(file_desc, &c, 1)) {
        if (c == '\n') {
            ++pos_in_str;
            (*str_len)[str_num] = pos_in_str ;
            ++str_num;
            (*offset)[str_num] = lseek(file_desc, 0, SEEK_CUR);
            if((*offset)[str_num] == -1){
                perror("Failed during lseek in get_file_info");
                return -1;
            }
            pos_in_str = 0;
            if(++(*amount_of_lines) == SIZE) {
                realloc(*str_len, max_size + SIZE);
                if((*str_len) == NULL){
                    perror("Failed to reallocate memory for str_len in get_file_info\n");
                    return -1;
                }
                realloc(*offset, max_size + SIZE);
                if((*offset) == NULL){
                    perror("Failed to reallocate memory for offset in get_file_info\n");
                    return -1;
                }
            }
        }
        else {
            ++pos_in_str;
        }
    }
   return 0;
}

int get_num(long *number) {
    char number_buff[SIZE];
    int bytes_read = read(STDIN_FILENO, number_buff, SIZE);
    if (bytes_read == -1) {
        perror("Could not read input number");
        return -1;
    }

    if (bytes_read == 0) {
        printf("No input to read number\n");
        return 1;
    }

    if (number_buff[bytes_read - 1] != '\n') {
        printf("Entered string is too long\n");
        return 1;
    }

    number_buff[bytes_read - 1] = '\0';

    char *end_ptr;
    char *buff_end_ptr = number_buff + bytes_read - 1;

    *number = strtol(number_buff, &end_ptr, 10);
    if (errno == ERANGE && (*number == LONG_MAX || *number == LONG_MIN)) {
        fprintf(stderr, "Number is outside the range  of  representable values\n");
        return -1;
    }

    for (char *c = end_ptr; c != buff_end_ptr; c++) {
        if (!isspace(*c)) {
            printf("Wrong number format, try again\n");
            return 1;
        }
    }
    return 0;
}



int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong amount of arguments, need 1");
        return -1;
    }
    int file_desc;
    if ((file_desc = open(argv[1], O_RDONLY)) == -1) {
        perror("can't open file");
        return -1;
    }

    int amount_of_lines;
    int *str_len;
    int *offset;
    char buf[SIZE];
    int err;
    if(get_file_info(file_desc, &amount_of_lines, &str_len, &offset) == -1){
       return -1;
    }

    long number;
    while (1) {
        printf("Write number of string\n");
        err = get_num(&number);
        if(err == 1){
            continue;
        }
        else if(err == -1){
            return -1;
        }

        if(number == 0){
            printf("See you next time!");
            break;
        }
        if (number < 0 || number > amount_of_lines) {
            printf("Number is outside the range  of  representable values. Amount of lines = %d. Try again.\n", amount_of_lines);
            continue;
        }
        if(lseek(file_desc, offset[number - 1], SEEK_SET) == -1){
            perror("Failed during lseek in main");
            return -1;
        }
        for(int j = 0; j < SIZE; j++){
            buf[j] = 0;
        }
        if (read(file_desc, buf, str_len[number - 1]) >= 0) {
            printf("%s\n", buf);
        } else {
            perror("Failed to read from file");
            return -1;
        }
    }
    close(file_desc);
    free(offset);
    free(str_len);
    return 0;
}
