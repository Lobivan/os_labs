#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define ARR_SIZE 2
#define BUF_SIZE 1024


int get_file_info(int file_desc, int *amount_of_lines, int **length_of_string, int **string_offset){
    *amount_of_lines = 0;
    int max_size = ARR_SIZE;
    (*length_of_string) = (int*) malloc(sizeof(int) * ARR_SIZE);
    if((*length_of_string) == NULL){
        perror("Failed to allocate memory for length_of_string in get_file_info\n");
        return -1;
    }
    (*string_offset) = (int*) malloc(sizeof(int) * ARR_SIZE);
    if((*string_offset) == NULL){
        perror("Failed to allocate memory for string_offset in get_file_info\n");
        return -1;
    }
    char c;
    int pos_in_str = 0;
    int str_num = 0;
    (*string_offset)[0] = 0;
    int error = 1;
    while (error == 1) {
        error = read(file_desc, &c, 1);
        if(error == -1){
            perror("Error while reading in get_file_info");
            return -1;
        }
        if (c == '\n') {
            ++pos_in_str;
            (*length_of_string)[str_num] = pos_in_str ;
            ++str_num;
            (*string_offset)[str_num] = lseek(file_desc, 0, SEEK_CUR);
            if((*string_offset)[str_num] == -1){
                perror("Failed during lseek in get_file_info");
                return -1;
            }
            pos_in_str = 0;
            if(++(*amount_of_lines) == ARR_SIZE) {
                max_size+=ARR_SIZE;
                (*length_of_string) = realloc(*length_of_string, max_size);
                if((*length_of_string) == NULL){
                    perror("Failed to reallocate memory for length_of_string in get_file_info\n");
                    return -1;
                }
                (*string_offset) = realloc(*string_offset, max_size);
                if((*string_offset) == NULL){
                    perror("Failed to reallocate memory for string_offset in get_file_info\n");
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
    char number_buff[BUF_SIZE];
    int bytes_read = read(0, number_buff, BUF_SIZE);
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
    int *length_of_string;
    int *string_offset;
    char buf[BUF_SIZE];
    int error;
    get_file_info(file_desc, &amount_of_lines, &length_of_string, &string_offset);

    long number;
    while (1) {
        printf("Write number of string\n");
        error = get_num(&number);
        if(error == 1){
            continue;
        }
        else if(error == -1){
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
        if(lseek(file_desc, string_offset[number - 1], SEEK_SET) == -1){
            perror("Failed during lseek in main");
            return -1;
        }
        for(int j = 0; j < BUF_SIZE; j++){
            buf[j] = 0;
        }
        error = read(file_desc, buf, length_of_string[number - 1]);
        if(error == -1){
            perror("Error while reading in get_file_info");
            return -1;
        }
        if (error > 0) {
            printf("%s\n", buf);
        } else {
            perror("Failed to read from file");
            return -1;
        }
    }
    close(file_desc);
    free(string_offset);
    free(length_of_string);
    return 0;
}
