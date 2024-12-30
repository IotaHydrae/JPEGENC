#include <stdio.h>

#include "../examples/rgb565_demo/rgb565.h"

void binary_array_to_file(char *path, unsigned char *data, size_t len)
{
    FILE *output;

    printf("%ld\n", len);

    output = fopen(path, "wb");
    fwrite(data, len, 1, output);
    fclose(output);
}

int main(int argc, char **argv)
{
    if (argc == 2)
        binary_array_to_file(argv[1], rgb565, sizeof(rgb565));

    return 0;
}
