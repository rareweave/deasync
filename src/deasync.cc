#include <uv.h>
#include <node_api.h>
#include <pthread.h>

// Global mutex and condition variable
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool async_work_done = false;

void async_work_callback(uv_work_t* req) {
    // ... Do your async work here ...

    // Signal that async work is done
    pthread_mutex_lock(&mutex);
    async_work_done = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

napi_value Run(napi_env env, napi_callback_info info) {
    uv_loop_s* loop;
    napi_get_uv_event_loop(env, &loop);

    uv_work_t work_req;
    uv_queue_work(loop, &work_req, async_work_callback, NULL);

    pthread_mutex_lock(&mutex);
    while (!async_work_done) {
        pthread_cond_wait(&cond, &mutex);
    }
    async_work_done = false; // reset the flag
    pthread_mutex_unlock(&mutex);

    return nullptr;
}

napi_value Init(napi_env env, napi_value exports) {
    napi_value fn_run;
    napi_create_function(env, "run", NAPI_AUTO_LENGTH, Run, NULL, &fn_run);

    napi_set_named_property(env, exports, "run", fn_run);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
