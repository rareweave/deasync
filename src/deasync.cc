#define NAPI_VERSION 3

#include <uv.h>
#include <node_api.h>

static int run_depth = 0;

napi_value Run(napi_env env, napi_callback_info info) {
    uv_loop_s* loop;
    napi_get_uv_event_loop(env, &loop);

    run_depth++;
    while (run_depth > 0) {
        uv_run(loop, UV_RUN_NOWAIT);
        run_depth--;
    }
    
    return nullptr;
}

napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_run;
    napi_create_function(env, "run", NAPI_AUTO_LENGTH, Run, NULL, &fn_run);

    napi_set_named_property(env, exports, "run", fn_run);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
