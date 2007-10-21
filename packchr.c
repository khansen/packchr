#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char program_version[] = "packchr 1.0";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: packchr [--nametable-base=NUM]\n"
        "               [--character-output=FILE] [--nametable-output=FILE]\n"
        "               [--help] [--usage] [--version]\n"
        "                FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: packchr [OPTION...] FILE\n\n"
           "  --nametable-base=NUM            Use NUM as nametable base\n"
           "  --character-output=FILE         Store packed CHR in FILE\n"
           "  --nametable-output=FILE         Store nametable in FILE\n"
           "  --help                          Give this help list\n"
           "  --usage                         Give a short usage message\n"
           "  --version                       Print program version\n");
    exit(0);
}

/* Prints version and exits. */
static void version()
{
    printf("%s\n", program_version);
    exit(0);
}

/**
 * Program entrypoint.
 */
int main(int argc, char **argv)
{
    char *chr_in;
    char *chr_out = 0;
    long sz_in;
    long sz_out;
    char nametable[1024];
    int nametable_sz;
    int nametable_base = 0;
    const char *input_filename = 0;
    const char *character_output_filename = 0;
    const char *nametable_output_filename = 0;
    /* Process arguments. */
    {
        char *p;
        while ((p = *(++argv))) {
            if (!strncmp("--", p, 2)) {
                const char *opt = &p[2];
                if (!strncmp("nametable-base=", opt, 15)) {
                    nametable_base = strtol(&opt[15], 0, 0);
                } else if (!strncmp("character-output=", opt, 17)) {
                    character_output_filename = &opt[17];
                } else if (!strncmp("nametable-output=", opt, 17)) {
                    nametable_output_filename = &opt[17];
                } else if (!strcmp("help", opt)) {
                    help();
                } else if (!strcmp("usage", opt)) {
                    usage();
                } else if (!strcmp("version", opt)) {
                    version();
                } else {
                    fprintf(stderr, "unrecognized option `%s'\n", p);
                    return(-1);
                }
            } else {
                input_filename = p;
            }
        }
    }

    if (!input_filename) {
        fprintf(stderr, "packchr: no filename given\n");
        return(-1);
    }

    {
        FILE *fp = fopen(input_filename, "rb");
        if (!fp) {
            fprintf(stderr, "packchr: failed to open `%s' for reading\n", input_filename);
            return(-1);
        }

        fseek(fp, 0, SEEK_END);
        sz_in = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        chr_in = (char *)malloc(sz_in);
        if (fread(chr_in, 1, sz_in, fp) != sz_in) {
            fprintf(stderr, "packchr: failed to read contents of `%s'\n", input_filename);
            return(-1);
        }
        fclose(fp);
    }

    if (!character_output_filename)
        character_output_filename = "packchr.chr";
    if (!nametable_output_filename)
        nametable_output_filename = "packchr.nam";

    {
        long buf_sz = 0;
        long pos_in = 0;
        long pos_out = 0;
        long nametable_pos = 0;
        while (pos_in < sz_in) {
            long i;
            char *tile_in = &chr_in[pos_in];
            for (i = 0; i < pos_out; i += 16) {
                if (!memcmp(tile_in, &chr_out[i], 16))
                    break;
            }
            if (i == pos_out) {
                if (pos_out == buf_sz) {
                    buf_sz += 1024;
                    chr_out = realloc(chr_out, buf_sz);
                }
                memcpy(&chr_out[pos_out], tile_in, 16);
                pos_out += 16;
            }
            nametable[nametable_pos++] = (char)((i / 16) + nametable_base);
            pos_in += 16;
        }
        sz_out = pos_out;
        nametable_sz = nametable_pos;
    }

    {
        FILE *fp = fopen(character_output_filename, "wb");
        fwrite(chr_out, 1, sz_out, fp);
        fclose(fp);
    }

    {
        FILE *fp = fopen(nametable_output_filename, "wb");
        fwrite(nametable, 1, nametable_sz, fp);
        fclose(fp);
    }

    free(chr_in);
    free(chr_out);

    return 0;
}
