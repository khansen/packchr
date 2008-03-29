#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static char program_version[] = "packchr 1.0";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: packchr [--nametable-base=NUM] [--null-tile=NUM]\n"
        "               [--character-output=FILE] [--nametable-output=FILE]\n"
        "               [--character-size=SIZE]\n"
        "               [--help] [--usage] [--version]\n"
        "                FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: packchr [OPTION...] FILE\n"
           "packchr finds the unique tiles in a NES character (CHR) file.\n\n"
           "Options:\n\n"
           "  --nametable-base=NUM            Use NUM as nametable base tile index;\n"
           "                                  i.e. the first tile will be referenced\n"
           "                                  as NUM in the nametable, the second as\n"
           "                                  NUM+1, etc.; by default NUM is 0\n"
           "  --null-tile=NUM                 Use NUM as implicit null tile index; that is,\n"
           "                                  don't produce any data for a 'blank' tile\n"
           "  --character-output=FILE         Store packed CHR in FILE\n"
           "  --character-size=SIZE           Pad to SIZE bytes if necessary\n"
           "  --nametable-output=FILE         Store nametable in FILE\n"
           "  --verbose                       Print statistics\n"  
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
    static const char null_tile_data[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    char *chr_in;
    char *chr_out = 0;
    long sz_in;
    long sz_out;
    unsigned char *nametable;
    int nametable_sz;
    int nametable_base = 0;
    const char *input_filename = 0;
    const char *character_output_filename = 0;
    const char *nametable_output_filename = 0;
    int null_tile = -1;
    long pad_sz = -1;
    int verbose = 0;
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
                } else if (!strncmp("character-size=", opt, 15)) {
                    pad_sz = strtol(&opt[15], 0, 0);
                } else if (!strncmp("nametable-output=", opt, 17)) {
                    nametable_output_filename = &opt[17];
                } else if (!strncmp("null-tile=", opt, 10)) {
                    null_tile = strtol(&opt[10], 0, 0);
                } else if (!strcmp("verbose", opt)) {
                    verbose = 1;
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
        fprintf(stderr, "packchr: no filename given\n"
                        "Try `packchr --help' or `packchr --usage' for more information.");
        return(-1);
    }

    /* Read CHR */
    if (verbose)
        fprintf(stdout, "reading `%s'\n", input_filename);
    {
        FILE *fp = fopen(input_filename, "rb");
        if (!fp) {
            fprintf(stderr, "packchr: failed to open `%s' for reading\n", input_filename);
            return(-1);
        }

        fseek(fp, 0, SEEK_END);
        sz_in = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if ((sz_in & 15) != 0) {
            fprintf(stderr, "packchr: warning: `%s' is not a multiple of 16 bytes\n", input_filename);
            sz_in &= ~15;
        }
        chr_in = (char *)malloc(sz_in);
        nametable_sz = sz_in / 16;
        nametable = (unsigned char *)malloc(nametable_sz);
        if (!chr_in || !nametable) {
            fprintf(stderr, "packchr: failed to allocate memory for `%s'\n", input_filename);
            return(-1);
        }
        if (fread(chr_in, 1, sz_in, fp) != sz_in) {
            fprintf(stderr, "packchr: failed to read contents of `%s'\n", input_filename);
            return(-1);
        }
        fclose(fp);
    }
    if (verbose)
        fprintf(stdout, "read %ld tile(s) (%ld bytes)\n", sz_in/16, sz_in);

    if (!character_output_filename)
        character_output_filename = "packchr.chr";
    if (!nametable_output_filename)
        nametable_output_filename = "packchr.nam";

    /* Create packed CHR and nametable */
    if (verbose)
        fprintf(stdout, "processing\n");
    {
        long buf_sz = 0;
        long pos_in = 0;
        long pos_out = 0;
        long nametable_pos = 0;
        while (pos_in < sz_in) {
            long i;
            const char *tile_in = &chr_in[pos_in];
            const int is_null_tile = !memcmp(tile_in, null_tile_data, 16);
            if (!is_null_tile || (null_tile == -1)) {
                /* See if tile is equal to one we already recorded */
                for (i = 0; i < pos_out; i += 16) {
                    if (!memcmp(tile_in, &chr_out[i], 16))
                        break;
                }
                if (i == pos_out) {
                    /* Add tile */
                    if (pos_out == buf_sz) {
                        buf_sz += 1024;
                        chr_out = realloc(chr_out, buf_sz);
                        assert(chr_out);
                    }
                    memcpy(&chr_out[pos_out], tile_in, 16);
                    pos_out += 16;
                }
                nametable[nametable_pos] = (unsigned char)((i / 16) + nametable_base);
            } else {
                nametable[nametable_pos] = (unsigned char)null_tile;
            }
            ++nametable_pos;
            pos_in += 16;
        }
        sz_out = pos_out;
        assert(nametable_sz == nametable_pos);
    }

    /* Write CHR */
    if (verbose) {
        fprintf(stdout, "writing CHR to `%s'; %ld tile(s) (%ld bytes)\n",
                character_output_filename, sz_out/16, sz_out);
    }
    {
        FILE *fp = fopen(character_output_filename, "wb");
        fwrite(chr_out, 1, sz_out, fp);
        if (pad_sz != -1) {
            if (pad_sz > sz_out) {
                long i;
                for (i = sz_out; i < pad_sz; ++i)
                    fputc(0, fp);
            }
        }
        fclose(fp);
    }

    /* Write nametable */
    if (verbose)
        fprintf(stdout, "writing nametable to `%s' (%d bytes)\n", nametable_output_filename, nametable_sz);
    {
        FILE *fp = fopen(nametable_output_filename, "wb");
        fwrite(nametable, 1, nametable_sz, fp);
        fclose(fp);
    }

    if (verbose)
        fprintf(stdout, "compressed size: %ld%%\n", (sz_out*100) / sz_in);

    free(chr_in);
    free(chr_out);
    free(nametable);

    /* Success */
    return 0;
}
