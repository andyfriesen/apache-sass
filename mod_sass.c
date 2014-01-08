
#include <httpd.h>
#include <http_config.h>
#include <http_core.h>
#include <http_protocol.h>
#include <http_request.h>

#include <apr_strings.h>

#include "sass_interface.h"

module AP_MODULE_DECLARE_DATA sass_module;

typedef struct _Config {
    char* include_path;
} Config;

int exists(const char* filename) {
    FILE* f;
    if (f = fopen(filename, "r")) {
        fclose(f);
        return 1;
    }
    return 0;
}

static int sass_handler(request_rec* r) {
    char* filename = apr_pstrdup(r->pool, r->canonical_filename);
    const int len = strlen(filename);
    int is_css = 0;

    if (len > 4) {
        const char* last4 = filename + len - 4;
        if (0 == apr_strnatcasecmp(last4, ".css")) {
            is_css = 1;
        }
    }

    if (!is_css && 0 != apr_strnatcasecmp(r->handler, "sass")) {
        return DECLINED;
    }

    if (M_GET != r->method_number) {
        return HTTP_METHOD_NOT_ALLOWED;
    }

    Config* config = ap_get_module_config(r->per_dir_config, &sass_module);

    if (is_css) {
        filename = apr_psprintf(r->pool, "%.*s.scss", len - 4, filename);

        if (!exists(filename)) {
            // Requested a .css file which has no corresponding .scss
            // The request will either produce the css file itself it exists, else 404.
            return DECLINED;
        }
    }

    struct sass_file_context* ctx = sass_new_file_context();
    ctx->input_path = filename;
    ctx->options.output_style = SASS_STYLE_EXPANDED;
    ctx->options.source_comments = SASS_SOURCE_COMMENTS_DEFAULT;
    ctx->options.include_paths = apr_pstrdup(r->pool, config->include_path);
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

    return OK;
}

static void register_hooks(apr_pool_t* pool) {
    ap_hook_handler(sass_handler, 0, 0, APR_HOOK_FIRST);
}

static const command_rec command_recs[] = {
    AP_INIT_TAKE1(
        "sassIncludePaths",
        ap_set_string_slot,
        (void*)APR_OFFSETOF(Config, include_path),
        RSRC_CONF|ACCESS_CONF,
        "SCSS include paths"
    ),
    {NULL}
};

void* make_config(apr_pool_t* pool, char* dir) {
    Config* config = apr_pcalloc(pool, sizeof(Config));
    config->include_path = "";
    return (void*)config;
}

void* merge_config(apr_pool_t* pool, void* _unused_baseConfig, void* overrideConfig) {
    Config* override = (Config*)overrideConfig;

    Config* result = apr_pcalloc(pool, sizeof(Config));
    result->include_path = override->include_path;
    return result;
}

module AP_MODULE_DECLARE_DATA sass_module = {
    STANDARD20_MODULE_STUFF,
    make_config,
    merge_config,
    0,
    0,
    command_recs,
    register_hooks
};
