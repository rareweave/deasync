#define NAPI_VERSION 3

#include <uv.h>
#include <node_api.h>

uv_async_t async_handle;

void async_cb(uv_async_t* handle) {
    // This is called when our Run function triggers the async handle.
    // We don't need to do anything here, it's just to wake up uv_run.
}

napi_value Run(napi_env env, napi_callback_info info) {
    uv_loop_s* loop;
    napi_get_uv_event_loop(env, &loop);

    // Send an async signal. This will trigger the async_cb.
    uv_async_send(&async_handle);

    uv_run(loop, UV_RUN_NOWAIT);
    return nullptr;
}

napi_value Init(napi_env env, napi_value exports) {
    uv_loop_s* loop;
    napi_get_uv_event_loop(env, &loop);

    // Initialize our async handle.
    uv_async_init(loop, &async_handle, async_cb);

    napi_value fn_run;
    napi_create_function(env, "run", NAPI_AUTO_LENGTH, Run, NULL, &fn_run);

    napi_set_named_property(env, exports, "run", fn_run);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
