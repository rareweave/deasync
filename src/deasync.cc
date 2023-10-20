#define NAPI_VERSION 3

#include <uv.h>
#include <node_api.h>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>  // for usleep

// Declare mutex, condition variable and async_work_done as global variables
pthread_mutex_t mutex;
bool async_work_done = false;

void async_work_callback(uv_work_t* req) {
    // Simulate async work with a sleep
    sleep(2);

    // Signal that async work is done
    pthread_mutex_lock(&mutex);
    async_work_done = true;
    pthread_mutex_unlock(&mutex);
    free(req);
}
uv_async_t async_handle;
volatile bool is_inside_uv_run = false;

void async_cb(uv_async_t* handle) {
    // This will be called when our async event is dispatched.
    // By calling uv_stop, we'll exit out of the uv_run loop.
    uv_stop(handle->loop);
}

napi_value Run(napi_env env, napi_callback_info info) {
    uv_loop_s* loop;
    napi_status status = napi_get_uv_event_loop(env, &loop);
    if (status != napi_ok) {
        return NULL;
    }

    uv_work_t *work_req = (uv_work_t *)malloc(sizeof(uv_work_t));
    if (work_req == NULL) {
        return NULL;  // Memory allocation failed
    }

    uv_queue_work(loop, work_req, async_work_callback, NULL);

    while (true) {
        pthread_mutex_lock(&mutex);
        if (async_work_done) {
            async_work_done = false;
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        // Give some time for other events in the event loop to be processed
        uv_run(loop, UV_RUN_NOWAIT);
        usleep(1000); // Sleep for 1ms to avoid 100% CPU usage
    }

    return NULL;
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
