#include "font4x6_basic.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		fputs("Usage: encode-font C-FILE H-FILE NAME\n", stderr);
        return EXIT_FAILURE;
	}

    const char* c_filename = argv[1];
    const char* h_filename = argv[2];
    const char* name = argv[3];

    FILE* file = fopen(h_filename, "w+");
    if (file == NULL)
    {
        fprintf(stderr, "Error creating file %s: %s\n", h_filename, strerror(errno));
        return EXIT_FAILURE;
    }
    fprintf(file,
        "#pragma once\n"
        "\n"
        "#include <stdint.h>\n"
        "\n"
        "extern const uint8_t %s[128][6];\n", name);
    fclose(file);

    file = fopen(c_filename, "w+");
    if (file == NULL)
    {
        fprintf(stderr, "Error creating file %s: %s\n", c_filename, strerror(errno));
        return EXIT_FAILURE;
    }
    fprintf(file,
        "#include <stdint.h>\n"
        "\n"
        "const uint8_t %s[128][6] = {\n", name);

    for (int i = 0; i != 128; ++i)
    {
        fputs("    {", file);
        for (int y = 0; y != 6; ++y)
        {
            uint8_t pixels = (font4x6_basic[i][y / 2] >> (y % 2 == 1 ? 0 : 4)) & 0x0F;
            uint8_t value = 0;
            for (int x = 0; x != 4; ++x)
            {
                value |= ((pixels >> x) & 0x1) << (x * 2);
            }

            if (y != 0)
            {
                fputs(", ", file);
            }
            fprintf(file, "0x%02hhX", value);
        }
        fprintf(file, "}%c // U+%04X\n", i + 1 != 128 ? ',' : ' ', i);
    }
    fputs("};\n", file);
    fclose(file);

	return EXIT_SUCCESS;
}
