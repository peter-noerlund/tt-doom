#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_chrominance_table(FILE* file, const char* name)
{
    fprintf(file, "const uint8_t %s[64] = {", name);

    for (uint8_t c = 0; c != 64; ++c)
    {
        if (c != 0)
        {
            fputc(',', file);
        }
        if (c % 16 == 0)
        {
            fputs("\n    ", file);
        }

        uint8_t r = (c >> 4);
        uint8_t g = (c >> 2) & 0x03;
        uint8_t b = c & 0x03;

        fprintf(file, "%2hhu", r * 3 + g * 7 + b);
    }

    fputs("\n};\n", file);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        fputs("Usage: create-tables C-FILE H-FILE\n", stderr);
        return EXIT_FAILURE;
    }

    const char* c_filename = argv[1];
    const char* h_filename = argv[2];

    FILE* file = fopen(h_filename, "w+");
    if (file == NULL)
    {
        fprintf(stderr, "Error creating file %s: %s\n", h_filename, strerror(errno));
        return EXIT_FAILURE;
    }
    fputs(
        "#pragma once\n"
        "\n"
        "#include <stdint.h>\n"
        "\n"
        "extern const uint8_t g_chrominance_table[64];\n",
        file
    );
    fclose(file);

    file = fopen(c_filename, "w+");
    if (file == NULL)
    {
        fprintf(stderr, "Error creating file %s: %s\n", c_filename, strerror(errno));
        return EXIT_FAILURE;
    }
    fputs(
        "#include <stdint.h>\n"
        "\n", file);

    write_chrominance_table(file, "g_chrominance_table");

    return EXIT_SUCCESS;
}
