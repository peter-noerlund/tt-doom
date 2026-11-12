#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        fputs("Usage: file-to-c FILE C-FILE H-FILE NAME\n", stderr);
        return EXIT_FAILURE;
    }

    const char* input_filename = argv[1];
    const char* c_filename = argv[2];
    const char* h_filename = argv[3];
    const char* name = argv[4];

    FILE* input_file = fopen(input_filename, "rb");
    if (input_file == NULL)
    {
        fprintf(stderr, "Error opening file %s: %s\n", input_filename, strerror(errno));
        return EXIT_FAILURE;
    }

    fseek(input_file, 0, SEEK_END);
    long int size = ftell(input_file);
    if (size < 0)
    {
        fprintf(stderr, "Error getting file size: %s\n", strerror(errno));
        fclose(input_file);
        return EXIT_FAILURE;
    }
    rewind(input_file);

    FILE* output_file = fopen(h_filename, "w+");
    if (output_file == NULL)
    {
        fprintf(stderr, "Error creating file %s: %s\n", h_filename, strerror(errno));
        fclose(input_file);
        return EXIT_FAILURE;
    }
    fprintf(output_file,
            "#pragma once\n"
            "\n"
            "#include <stdint.h>\n"
            "\n"
            "extern const uint8_t %s[%li];\n", name, size);
    fclose(output_file);

    output_file = fopen(c_filename, "w+");
    if (output_file == NULL)
    {
        fprintf(stderr, "Error creating file %s: %s\n", h_filename, strerror(errno));
        fclose(input_file);
        return EXIT_FAILURE;
    }
    fprintf(output_file,
            "#include <stdint.h>\n"
            "\n"
            "const uint8_t %s[%li] = {\n", name, size);

    char* buffer = (char*)malloc(512);
    while (size != 0)
    {
        long int chunk_size = (size > 512 ? 512 : size);
        if (fread(buffer, chunk_size, 1, input_file) != 1)
        {
            fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
            free(buffer);
            fclose(input_file);
            fclose(output_file);
            return EXIT_FAILURE;
        }

        size -= chunk_size;

        fputs("    ", output_file);
        for (long int i = 0; i != chunk_size; ++i)
        {
            if (i != 0)
            {
                fputc(',', output_file);
            }
            fprintf(output_file, "0x%02hhX", buffer[i]);
        }
        if (size != 0)
        {
            fputc(',', output_file);
        }
        fputc('\n', output_file);
    }
    fputs("};\n", output_file);

    free(buffer);
    fclose(input_file);
    fclose(output_file);

    return EXIT_SUCCESS;
}
