
#include <httpd.h>
#include <http_config.h>
#include <http_core.h>
#include <http_protocol.h>
#include <http_request.h>

#include <apr_strings.h>

#include "sass_interface.h"

typedef struct _Config {
    char* include_path;
} Config;

Config config;

apr_pool_t* pool = NULL;

apr_pool_t* getPool() {
    if (pool == NULL) {
        apr_pool_create(&pool, NULL);
    }
    return pool;
}

void clearPool() {
    if (pool != NULL) {
        apr_pool_destroy(pool);
        pool = NULL;
    }
}

int exists(const char* filename) {
    FILE* f;
    if (f = fopen(filename, "r")) {
        fclose(f);
        return 1;
    }
    return 0;
}

static int sass_handler(request_rec* r) {
    if (0 != apr_strnatcasecmp(r->handler, "sass")) {
        return DECLINED;
    }

    if (M_GET != r->method_number) {
        return HTTP_METHOD_NOT_ALLOWED;
    }

    size_t len = strlen(r->filename);
    char* filename = apr_palloc(getPool(), len + 1);
    strcpy(filename, r->filename);
    filename[len] = 0;

    if (len > 4) {
        const char* last4 = r->filename + len - 4;

        if (0 == apr_strnatcasecmp(".css", last4)) {
            strcpy(filename + len - 4, ".scss\0");

            if (!exists(filename)) {
                // Requested a .css file which has no corresponding .scss
                // The request will either produce the css file itself it exists, else 404.
                ap_set_content_type(r, "text/plain");
                ap_rprintf(r, "aeuaoeuaoe\n");
                return OK;

                return DECLINED;
            }
        }
    }

    struct sass_file_context* ctx = sass_new_file_context();
    ctx->input_path = filename;
    ctx->options.output_style = SASS_STYLE_EXPANDED;
    ctx->options.source_comments = SASS_SOURCE_COMMENTS_DEFAULT;
    ctx->options.include_paths = config.include_path;
    ctx->options.image_path = ".";

    sass_compile_file(ctx);

    if (ctx->error_status) {
        ap_set_content_type(r, "text/plain");
        ap_rprintf(r, "%s\n", ctx->error_message);
        return OK;
    }


    ap_set_content_type(r, "text/css");
    ap_rprintf(r, "%s", ctx->output_string);

    sass_free_file_context(ctx);

    clearPool();

    return OK;
}

static void register_hooks(apr_pool_t* pool) {
    ap_hook_handler(sass_handler, 0, 0, APR_HOOK_LAST);
}

static const char* set_include_path(cmd_parms* cmd, void* cfg, const char* arg) {
    config.include_path = apr_pstrdup(getPool(), arg);
    return NULL;
}

static const command_rec command_recs[] = {
    AP_INIT_TAKE1("includePaths", set_include_path, NULL, RSRC_CONF, "SCSS include paths"),
    {NULL}
};

module AP_MODULE_DECLARE_DATA sass_module = {
    STANDARD20_MODULE_STUFF,
    0,
    0,
    0,
    0,
    command_recs,
    register_hooks
};
