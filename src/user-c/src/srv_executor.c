#include <stdlib.h>
#include <string.h>

#include "api/sk_utils.h"
#include "idl_internal.h"
#include "txn_utils.h"
#include "srv_types.h"
#include "srv_executor.h"
#include "skull/service_loader.h"

void skull_srv_init (sk_service_t* srv, void* data)
{
    skull_service_opt_t* opt = data;

    skull_service_t skull_service = {
        .service = srv,
        .txn     = NULL,
        .task    = NULL,
        .freezed = 0
    };

    opt->init(&skull_service, opt->ud);
}

void skull_srv_release (sk_service_t* srv, void* data)
{
    skull_service_opt_t* opt = data;

    skull_service_t skull_service = {
        .service = srv,
        .txn     = NULL,
        .task    = NULL,
        .freezed = 0
    };

    opt->release(&skull_service, opt->ud);
}

/**
 * Invoke the real user service api function
 * Notes: This api can be ran on worker/bio
 */
int  skull_srv_iocall  (sk_service_t* srv, const sk_txn_t* txn, void* sdata,
                        sk_txn_taskdata_t* task_data, const char* api_name,
                        sk_srv_io_status_t ustatus)
{
    SK_ASSERT(task_data);

    // find the api func
    skull_service_opt_t* opt = sdata;

    const sk_service_api_t* api = sk_service_api(srv, api_name);
    if (!api) {
        return 1;
    }

    // invoke the user service api
    skull_service_t skull_service = {
        .service = srv,
        .txn     = txn,
        .task    = task_data,
        .freezed = 0
    };

    return opt->iocall(&skull_service, api_name, opt->ud);
}

/**
 * Run api callback
 * Notes: This api only can be ran on worker
 */
int skull_srv_iocall_complete(sk_service_t* srv, sk_txn_t* txn, void* sdata,
                                uint64_t task_id, const char* api_name)
{
    // get the task data
    sk_txn_taskdata_t* task_data = sk_txn_taskdata(txn, task_id);
    SK_ASSERT(task_data);

    if (!task_data->cb) {
        sk_print("no service api callback function to run, skip it\n");
        return 0;
    }

    skull_txn_t skull_txn;
    skull_txn_init(&skull_txn, txn);

    // Invoke task callback
    int ret = ((skull_svc_api_cb)task_data->cb)(&skull_txn,
                                    task_data->request, task_data->request_sz,
                                    task_data->response, task_data->response_sz);

    skull_txn_release(&skull_txn, txn);
    return ret;
}
