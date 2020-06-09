#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#define ARR_SIZE 1024
#define BUF_SIZE 1024
#define STDIN_FD 0
#define EXIT_AFTER_ERROR -1
#define CONTINUE_AFTER_ERROR 1
#define DEMICAL 10

typedef struct File_info{
    int file_desc;
    int amount_of_lines;
    int *length_of_string;
    int *string_offset;
} File_info;

int get_file_info(File_info *file_info){
    file_info->amount_of_lines = 0;
    int max_size = ARR_SIZE;
    (file_info->length_of_string) = (int*) malloc(sizeof(int) * ARR_SIZE);
    if((file_info->length_of_string) == NULL){
        perror("Failed to allocate memory for length_of_string in get_file_info\n");
        return EXIT_AFTER_ERROR;
    }
    (file_info->string_offset) = (int*) malloc(sizeof(int) * ARR_SIZE);
    if((file_info->string_offset) == NULL){
        perror("Failed to allocate memory for string_offset in get_file_info\n");
        return EXIT_AFTER_ERROR;
    }
    char c = '0';
    int pos_in_str = 0;
    int str_num = 0;
    (file_info->string_offset)[0] = 0;
    int error = 1;
    int *temp_ptr;
    while (error == 1) {
        error = read(file_info->file_desc, &c, 1);
        if(error == -1){
            perror("Error while reading in get_file_info");
            return EXIT_AFTER_ERROR;
        }
        if (c == '\n') {
            if(++(file_info->amount_of_lines) == max_size) {
                max_size += ARR_SIZE;
                temp_ptr = (int*)realloc(file_info->length_of_string, sizeof(int) * max_size);
                if(temp_ptr == NULL){
                    perror("Failed to reallocate memory for length_of_string in get_file_info\n");
                    return EXIT_AFTER_ERROR;
                }
                file_info->length_of_string = temp_ptr;
                temp_ptr = (int*)realloc(file_info->string_offset, sizeof(int) * max_size);
                if(temp_ptr == NULL){
                    perror("Failed to reallocate memory for string_offset in get_file_info\n");
                    return EXIT_AFTER_ERROR;
                }
                file_info->string_offset = temp_ptr;
            }
            ++pos_in_str;
            (file_info->length_of_string)[str_num] = pos_in_str ;
            ++str_num;
            (file_info->string_offset)[str_num] = lseek(file_info->file_desc, 0, SEEK_CUR);
            if((file_info->string_offset)[str_num] == -1){
                perror("Failed during lseek in get_file_info");
                return EXIT_AFTER_ERROR;
            }
            pos_in_str = 0;

        }
        else {
            ++pos_in_str;
        }
    }
    free(temp_ptr);
    return 0;
}

int get_num(long *number) {
    char number_buff[BUF_SIZE];
    int bytes_read = read(STDIN_FD, number_buff, BUF_SIZE);
    if (bytes_read == -1) {
        perror("Could not read input number");
        return EXIT_AFTER_ERROR;
    }

    if (bytes_read == 0) {
        printf("No input to read number\n");
        return CONTINUE_AFTER_ERROR;
    }

    if (number_buff[bytes_read - 1] != '\n') {
        printf("Entered string is too long\n");
        return CONTINUE_AFTER_ERROR;
    }

    number_buff[bytes_read - 1] = '\0';

    char *end_ptr;
    char *buff_end_ptr = number_buff + bytes_read - 1;

    *number = strtol(number_buff, &end_ptr, DEMICAL);
    if (errno == ERANGE && (*number == LONG_MAX || *number == LONG_MIN)) {
        fprintf(stderr, "Number is outside the range  of  representable values\n");
        return EXIT_AFTER_ERROR;
    }

    for (char *c = end_ptr; c != buff_end_ptr; c++) {
        if (!isspace(*c)) {
            printf("Wrong number format, try again\n");
            return CONTINUE_AFTER_ERROR;
        }
    }
    return 0;
}



int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong amount of arguments, need 1");
        return EXIT_AFTER_ERROR;
    }
    File_info file_info;
    if ((file_info.file_desc = open(argv[1], O_RDONLY)) == -1) {
        perror("can't open file");
        return EXIT_AFTER_ERROR;
    }

    char buf[BUF_SIZE];
    int error;
    if(get_file_info(&file_info)){
        close(file_info.file_desc);
        free(file_info.string_offset);
        free(file_info.length_of_string);
        return EXIT_AFTER_ERROR;
    }

    long number;
    while (1) {
        printf("Write number of string\n");
        error = get_num(&number);
        if(error == CONTINUE_AFTER_ERROR){
            continue;
        }
        else if(error == EXIT_AFTER_ERROR){
            return EXIT_AFTER_ERROR;
        }

        if(number == 0){
            printf("See you next time!");
            break;
        }
        if (number < 0 || number > file_info.amount_of_lines) {
            printf("Number is outside the range  of  representable values. Amount of lines = %d. Try again.\n", file_info.amount_of_lines);
            continue;
        }
        if(lseek(file_info.file_desc, file_info.string_offset[number - 1], SEEK_SET) == -1){
            perror("Failed during lseek in main");
            return EXIT_AFTER_ERROR;
        }
        for(int j = 0; j < BUF_SIZE; j++){
            buf[j] = 0;
        }
        error = read(file_info.file_desc, buf, file_info.length_of_string[number - 1]);
        if(error == -1){
            perror("Error while reading in main");
            return EXIT_AFTER_ERROR;
        }
        if (error >= 0) {
            printf("%s\n", buf);
        } else {
            perror("Failed to read from file");
            return EXIT_AFTER_ERROR;
        }
    }
    close(file_info.file_desc);
    free(file_info.string_offset);
    free(file_info.length_of_string);
    return 0;
}
