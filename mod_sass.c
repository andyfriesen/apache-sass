
#include <httpd.h>
#include <http_config.h>
#include <http_core.h>
#include <http_protocol.h>
#include <http_request.h>

#include <apr_strings.h>

#include "sass_interface.h"

static int sass_handler(request_rec* r) {
    if (0 != apr_strnatcasecmp(r->handler, "sass")) {
        return DECLINED;
    }

    if (M_GET != r->method_number) {
        return HTTP_METHOD_NOT_ALLOWED;
    }

    if (APR_NOFILE == r->finfo.filetype) {
        return DECLINED;
    }


    struct sass_file_context* ctx = sass_new_file_context();
    ctx->input_path = r->filename;
    ctx->options.output_style = SASS_STYLE_EXPANDED;
    ctx->options.source_comments = SASS_SOURCE_COMMENTS_DEFAULT;
    ctx->options.include_paths = "";
    ctx->options.image_path = ".";

    sass_compile_file(ctx);

    if (ctx->error_status) {
        ap_set_content_type(r, "text/css");
        ap_rprintf(r, "%s\n", ctx->error_message);
        return OK;
    }

    //ap_log_error(__FILE__, __LINE__, 0, 0, 0, 0, "SNTHSNTH");
    ap_rprintf(r, "%s", ctx->output_string);

    sass_free_file_context(ctx);

    return OK;
}

static void register_hooks(apr_pool_t* pool) {
    ap_hook_handler(sass_handler, 0, 0, APR_HOOK_LAST);
}

module AP_MODULE_DECLARE_DATA sass_module = {
    STANDARD20_MODULE_STUFF,
    0,
    0,
    0,
    0,
    0,
    register_hooks
};
