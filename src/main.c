
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "common.h"

int main(int argc, char *argv[])
{
    char *result;

    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <image>\n", argv[0]);
        return -1;
    }

    result = slashdot_decode(argv[1]);
    if(!result)
        return -1;

    printf("%s\n", result);

    return 0;
}

