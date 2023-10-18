#define NAPI_VERSION 3

#include <uv.h>
#include <node_api.h>

static uv_async_t async_handle;

void async_cb(uv_async_t* handle) {
    uv_loop_t* loop = handle->loop;
    while (uv_run(loop, UV_RUN_NOWAIT) != 0); // Run the loop until no more tasks are pending
}

napi_value Run(napi_env env, napi_callback_info info) {
    uv_loop_s* loop;
    napi_get_uv_event_loop(env, &loop);

    // Initialize the async handle if not already initialized
    if (!uv_is_active((uv_handle_t*)&async_handle)) {
        uv_async_init(loop, &async_handle, async_cb);
    }

    uv_async_send(&async_handle); // Trigger the async callback which runs the loop
    return nullptr;
}

napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_run;
    napi_create_function(env, "run", NAPI_AUTO_LENGTH, Run, NULL, &fn_run);
    napi_set_named_property(env, exports, "run", fn_run);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
