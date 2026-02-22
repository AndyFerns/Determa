#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef VERSION_FILE
#define VERSION_FILE "../include/version.h"
#endif

#define MAX_LINE_LEN 512
#define MAX_NAME_LEN 128

typedef struct Version {
    int major;
    int minor;
    int patch;
    char name[MAX_NAME_LEN];
} Version;

// Utility Functions

void die(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

void print_help(void) {
    printf("Version Bumper Tool\n\n");
    printf("Usage:\n");
    printf("  bump_version --major\n");
    printf("  bump_version --minor\n");
    printf("  bump_version --patch\n");
    printf("  bump_version --set X.Y.Z\n");
    printf("  bump_version --name \"Codename\"\n");
    printf("  bump_version --print\n");
    printf("  bump_version --help\n");
}

// Parsing logic

int parse_line(const char *line, Version *v) {
    if (sscanf(line, "#define VERSION_MAJOR %d", &v->major) == 1)
        return 1;

    if (sscanf(line, "#define VERSION_MINOR %d", &v->minor) == 1)
        return 1;

    if (sscanf(line, "#define VERSION_PATCH %d", &v->patch) == 1)
        return 1;

    if (sscanf(line, "#define VERSION_NAME \"%127[^\"]\"", v->name) == 1)
        return 1;

    return 0;
}


void read_version_file(Version *v) {
    FILE *file = fopen(VERSION_FILE, "r");
    if (!file)
        die("Could not open version.h");

    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), file)) {
        parse_line(line, v);
    }

    fclose(file);
}


/* ============================= */
/* Version Operations            */
/* ============================= */

void bump_major(Version *v) {
    v->major++;
    v->minor = 0;
    v->patch = 0;
}

void bump_minor(Version *v) {
    v->minor++;
    v->patch = 0;
}

void bump_patch(Version *v) {
    v->patch++;
}

void set_version(Version *v, const char *str) {
    if (sscanf(str, "%d.%d.%d", &v->major, &v->minor, &v->patch) != 3)
        die("Invalid version format. Use X.Y.Z");
}

void set_name(Version *v, const char *name) {
    strncpy(v->name, name, MAX_NAME_LEN - 1);
    v->name[MAX_NAME_LEN - 1] = '\0';
}


/* ============================= */
/* Writing Logic                 */
/* ============================= */

void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void write_version_file(const Version *v) {
    FILE *out = fopen(VERSION_FILE, "w");
    if (!out)
        die("Could not write version.h");

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    fprintf(out,
        "/**\n"
        " * @file version.h\n"
        " * @author Andrew Fernandes\n"
        " * @brief Auto-generated version header\n"
        " * @generated %s\n"
        " */\n\n"
        "#ifndef VERSION_H\n"
        "#define VERSION_H\n\n"
        "#define VERSION_MAJOR %d\n"
        "#define VERSION_MINOR %d\n"
        "#define VERSION_PATCH %d\n"
        "#define VERSION_NAME  \"%s\"\n"
        "#define VERSION_BUILD_TIMESTAMP \"%s\"\n\n"
        "#define VERSION_FULL \"v%d.%d.%d '%s'\"\n\n"
        "#endif // VERSION_H\n",
        timestamp,
        v->major, v->minor, v->patch,
        v->name,
        timestamp,
        v->major, v->minor, v->patch, v->name
    );

    fclose(out);
}


/* ============================= */
/* CLI Processing                */
/* ============================= */

void process_args(int argc, char *argv[], Version *v) {
    if (argc < 2) {
        print_help();
        exit(0);
    }

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--major") == 0) {
            bump_major(v);
        }
        else if (strcmp(argv[i], "--minor") == 0) {
            bump_minor(v);
        }
        else if (strcmp(argv[i], "--patch") == 0) {
            bump_patch(v);
        }
        else if (strcmp(argv[i], "--set") == 0) {
            if (i + 1 >= argc)
                die("--set requires X.Y.Z");
            set_version(v, argv[++i]);
        }
        else if (strcmp(argv[i], "--name") == 0) {
            if (i + 1 >= argc)
                die("--name requires a string");
            set_name(v, argv[++i]);
        }
        else if (strcmp(argv[i], "--print") == 0) {
            printf("Current Version: v%d.%d.%d '%s'\n",
                   v->major, v->minor, v->patch, v->name);
            exit(0);
        }
        else if (strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        }
        else {
            die("Unknown argument. Use --help");
        }
    }
}


int main(int argc, char *argv[]) {

    Version v = {0};

    read_version_file(&v);
    process_args(argc, argv, &v);
    write_version_file(&v);

    printf("Updated to v%d.%d.%d '%s'\n",
           v.major, v.minor, v.patch, v.name);

    return 0;
}