#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


static char s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";


// arg is length_of_key
int main(int argc, char* argv[])
{

    // check for input errors
    if (argc > 2) {
        fprintf(stderr, "too many arguments given to keygen\n");
        return 1;
    }
    else if (argc < 2) {
        fprintf(stderr, "no arguments given to keygen\n");
        return 1;
    }
    else {
        for (int k = 0; k < strlen(argv[1]); k++) {
            if (isdigit((unsigned char)argv[1][k]) == 0) {
                fprintf(stderr, "here is atoi input: %d\n", atoi(argv[1]));
                fprintf(stderr, "invalid input to keygen, please input an int\n");
                return 1;
            }
        }
        if (atoi(argv[1]) < 1) {
            fprintf(stderr, "here is atoi input: %d\n", atoi(argv[1]));
            fprintf(stderr, "enter a larger int for keygen\n");
            return 1;
        }

    }

    // generate a random key and send it to stdout
    // random values are generated via dev/urandom file of Unix
    FILE *randfp = fopen("/dev/urandom", "r");
    if (!randfp) exit(1);
    
    for (int i = 0; i < atoi(argv[1]); i++) {
        unsigned long long r;
        fread(&r, sizeof r, 1, randfp);
        r %= sizeof s - 1;
        printf("%c", s[r]);
    }
    putchar('\n');
    fclose(randfp);

    return 0;
}
