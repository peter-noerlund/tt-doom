#include <errno.h>
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

        fprintf(file, " %2hhu", r * 3 + g * 7 + b);
    }

    fputs("\n};\n", file);
}

static void write_chrominance_division_table(FILE* file, const char* name)
{
    fprintf(file, "const uint8_t %s[265][8] = {", name);

    for (uint16_t c = 0; c != 265; c++)
    {
        if (c != 0)
        {
            fputc(',', file);
        }
        fputs("\n    {", file);

        for (uint16_t count = 1; count != 9; ++count)
        {
            if (count != 1)
            {
                fputc(',', file);
            }
            uint16_t res = c / count;
            if (res > 33)
            {
                res = 99;
            }
            fprintf(file, " %2hu", res);
        }

        fputc('}', file);
    }

    fputs("\n};\n", file);
}


inline uint8_t diff(uint8_t a, uint8_t b)
{
    return a < b ? b - a : a - b;
}

static uint8_t color_distance(uint8_t c1, uint8_t c2)
{
    uint8_t r1 = c1 >> 4;
    uint8_t r2 = c2 >> 4;
    uint8_t g1 = (c1 >> 2) & 0x03;
    uint8_t g2 = (c2 >> 2) & 0x03;
    uint8_t b1 = c1 & 0x03;
    uint8_t b2 = c2 & 0x03;

    uint8_t rd = diff(r1, r2);
    uint8_t gd = diff(g1, g2);
    uint8_t bd = diff(b1, b2);

    return rd * rd + gd * gd + bd * bd;
}

static void write_color_distance_table(FILE* file, const char* name)
{
    fprintf(file, "const uint8_t %s[64][64] = {", name);

    for (uint8_t c1 = 0; c1 != 64; ++c1)
    {
        if (c1 != 0)
        {
            fputc(',', file);
        }
        fputs("\n    {", file);

        for (uint8_t c2 = 0; c2 != 64; ++c2)
        {
            if (c2 != 0)
            {
                fputc(',', file);
            }
            fprintf(file, " %2hu", color_distance(c1, c2));
        }

        fputc('}', file);
    }

    fputs("\n};\n", file);
}

static void write_summable_table(FILE* file, const char* name)
{
    fprintf(file, "const uint16_t %s[64] = {", name);

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

        uint16_t r = (uint16_t)(c & 0x30) << 6;         // --rr ---- -> -000 rr-- ---- ----
        uint16_t g = (uint16_t)(c & 0x0C) << 3;         // ---- gg-- -> ---- --00 0gg- ----
        uint16_t b = (uint16_t)(c & 0x03);              // ---- --bb -> ---- ---- ---0 00bb

        fprintf(file, " 0x%04hX", r | g | b);
    }

    fputs("\n};\n", file);
}

static void write_summable_division_table(FILE* file, const char* name)
{
    fprintf(file, "const uint8_t %s[32768][16] = {", name);

    for (uint16_t c = 0; c != 32768; ++c)
    {
        fputs("\n    {", file);

        uint8_t r = (c >> 10) & 0x1F;
        uint8_t g = (c >> 5) & 0x1F;
        uint8_t b = (c & 0x1F);

        for (uint8_t div = 1; div != 17; ++div)
        {
            if (div != 1)
            {
                fputc(',', file);
            }

            uint8_t avg_r = (r / div);
            uint8_t avg_g = (g / div);
            uint8_t avg_b = (b / div);

            if (avg_r > 3 || avg_g > 3 || avg_b > 3)
            {
                fputs(" 0xFF", file);
            }
            else
            {
                fprintf(file, " 0x%02X", (avg_r << 4) | (avg_g << 2) | avg_b);
            }
        }

        fputc('}', file);

        if (c + 1 != 32768)
        {
            fputc(',', file);
        }

        fprintf(file, " // 0x%04hX (r=%hhu g=%hhu b=%hhu)", c, r, g, b);
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
        "extern const uint8_t g_chrominance_table[64]; // Convert RGB222 to chrominance\n"
        "extern const uint8_t g_chrominance_division_table[265][8]; // Convert RGB222 to chrominance\n"
        "extern const uint8_t g_color_distance_table[64][64]; // Calculate distance between two RGB222 colors\n"
        "extern const uint16_t g_summable_table[64]; // Expand RGB222 to RGB555 without adjusting color\n"
        "extern const uint8_t g_summable_division_table[32768][16]; // Divide RGB555 with value (index 0 is 1)\n",
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
    write_chrominance_division_table(file, "g_chrominance_division_table");
    write_color_distance_table(file, "g_color_distance_table");
    write_summable_table(file, "g_summable_table");
    write_summable_division_table(file, "g_summable_division_table");

    return EXIT_SUCCESS;
}
