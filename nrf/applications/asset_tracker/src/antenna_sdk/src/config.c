#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "iotex_emb.h"


#if NOT_PLATFORM_NRF9160
/* Global context */
static iotex_st_config __g_config = {};

/* Supported API Version list */
static uint32_t __g_support_ver_list[] = {1};

static const char *__g_cert_files[] = {

    "/etc/ssl/certs/ca-certificates.crt",                // Debian/Ubuntu/Gentoo etc.
    "/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora/RHEL 6
    "/etc/ssl/ca-bundle.pem",                            // OpenSUSE
    "/etc/pki/tls/cacert.pem",                           // OpenELEC
    "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // CentOS/RHEL 7
    "/etc/ssl/cert.pem",                                 // Alpine Linux, macOS
    NULL
};


static const char *__g_cert_dirs[] = {

    "/etc/ssl/certs",               // SLES10/SLES11, https://golang.org/issue/12139
    "/system/etc/security/cacerts", // Android
    "/usr/local/share/certs",       // FreeBSD
    "/etc/pki/tls/certs",           // Fedora/RHEL
    "/etc/openssl/certs",           // NetBSD
    "/var/ssl/certs",               // AIX,
    NULL
};


/* Get file type */
static mode_t __get_file_type(const char *path) {
    struct stat attr;
    memset(&attr, 0, sizeof(attr));

    if (stat(path, &attr) != 0) {
        return -1;
    }

    return attr.st_mode;
}

/* Search cert file */
static const char *config_search_cert_file() {

    const char **file = NULL;

    for (file = __g_cert_files; *file; file++) {
        if (S_ISREG(__get_file_type(*file))) {
            return *file;
        }
    }

    return NULL;
}

/* Search cert dir */
static const char *config_search_cert_dir() {

    const char **dir = NULL;

    for (dir = __g_cert_dirs; *dir; dir++) {
        if (S_ISDIR(__get_file_type(*dir))) {
            return *dir;
        }
    }

    return NULL;
}

struct iotex_st_config get_config() {
    return __g_config;
}

void print_config() {

    iotex_st_config context = get_config();

    fprintf(stdout,
            "API Version: %d\n"
            "Cert dir: %s, Cert file: %s\n",
            context.ver, context.cert_dir, context.cert_file);
}
#endif

void clear_config(void) {
#if NOT_PLATFORM_NRF9160    
    memset(&__g_config, 0, sizeof(__g_config));
#endif    
}

/* If don't specify context, auto search cert file and directory */
int init_config(const struct iotex_st_config *config) {
#if NOT_PLATFORM_NRF9160
    size_t i;
    int version_supported = 0;

    if (config) {

        /* Use specified context */
        memcpy(&__g_config, config, sizeof(__g_config));

        if (!__g_config.ver) {
            __g_config.ver = 1;
        }

        if (!__g_config.cert_dir) {
            __g_config.cert_dir = config_search_cert_dir();
        }

        if (!__g_config.cert_file) {
            __g_config.cert_file = config_search_cert_file();
        }
    }
    else {

        __g_config.ver = 1;
        __g_config.cert_dir = config_search_cert_dir();
        __g_config.cert_file = config_search_cert_file();
    }

    /* Check api version */
    for (i = 0; i < sizeof(__g_support_ver_list) / sizeof(__g_support_ver_list[0]); i++) {
        if (__g_config.ver == __g_support_ver_list[i]) {
            version_supported = 1;
            break;
        }
    }

    if (!version_supported) {
        fprintf(stderr, "Unsupported API Version: %d\n", __g_config.ver);
        clear_config();
        return -IOTEX_E_VER;
    }

    /* Check cert file and directory */
    if (!S_ISDIR(__get_file_type(__g_config.cert_dir)) && !S_ISREG(__get_file_type(__g_config.cert_file))) {
        fprintf(stderr, "Do not found cert file or directory, please specify one!\n");
        clear_config();
        return -IOTEX_E_NOERR;
    }

    if (!config) {
        print_config();
    }
#endif
    return 0;
}
