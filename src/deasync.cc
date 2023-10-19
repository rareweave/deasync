#define NAPI_VERSION 3

#include <uv.h>
#include <node_api.h>

uv_async_t async_handle;
volatile bool is_inside_uv_run = false;

void async_cb(uv_async_t* handle) {
    // This will be called when our async event is dispatched.
    // By calling uv_stop, we'll exit out of the uv_run loop.
    uv_stop(handle->loop);
}

napi_value Run(napi_env env, napi_callback_info info) {
    uv_loop_s* loop;
    napi_get_uv_event_loop(env, &loop);

    if (is_inside_uv_run) {
        // If already inside uv_run, trigger our async event
        // to make sure the loop doesn't block indefinitely.
        uv_async_send(&async_handle);
        return nullptr;
    }

    is_inside_uv_run = true;
    uv_async_send(&async_handle); // This will make sure there's always something in the event queue.
    uv_run(loop, UV_RUN_ONCE);    // This will block until the above async send completes.
    is_inside_uv_run = false;

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
