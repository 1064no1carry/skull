#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <map>
#include <google/protobuf/descriptor.h>

#include "module_loader.h"
#include "srv_loader.h"
#include "srv_utils.h"
#include "txn_idldata.h"

#include "skullcpp/unittest.h"

namespace skullcpp {

using namespace google::protobuf;

UTModule::UTModule(const std::string& moduleName, const std::string& idlName,
                   const std::string& configName) {
    // 1. Create skullut_module env
    skull_module_loader_t loader = module_getloader();
    this->utModule_ = skullut_module_create(moduleName.c_str(), idlName.c_str(),
                                            configName.c_str(), "cpp", loader);

    // 2. fill basic fields
    this->idlName_ = idlName;
    this->msg_     = NULL;
}

UTModule::~UTModule() {
    // 1. Destroy msg
    if (this->msg_) {
        delete this->msg_;
        this->msg_ = NULL;
    }

    // 2. Destroy rawData
    TxnSharedRawData* rawData =
        (TxnSharedRawData*)skullut_module_data(this->utModule_);
    delete rawData;

    // 3. Destroy mock services
    MockSvcs::iterator it = this->mockSvcs_.begin();
    for (; it != this->mockSvcs_.end(); it++) {
        delete it->second;
    }

    // 4.
    skullut_module_destroy(this->utModule_);
}

bool UTModule::run() {
    return skullut_module_run(this->utModule_);
}

// Fill the task.response
static
void _iocall(const char* apiName, skullmock_task_t* task, void* ud) {
    UTModule* utModule = (UTModule*)ud;
    std::string svcName(task->service->name);

    // prepare apiReq and apiResp
    ServiceApiReqData apiReq(task);
    const google::protobuf::Message* respMsg = utModule->popServiceCall(svcName, apiName);
    if (respMsg) {
        respMsg->PrintDebugString();
    }

    // Fill task.response
    if (!respMsg) {
        return;
    }

    task->response_sz = (size_t)respMsg->ByteSize();
    task->response = calloc(1, task->response_sz);
    respMsg->SerializeToArray(task->response, (int)task->response_sz);
}

static
void _release(void* ud) {
}

const google::protobuf::Message* UTModule::popServiceCall(
        const std::string& svcName,
        const std::string& apiName) {
    MockApiCall* apis = NULL;
    MockSvcs::iterator it = this->mockSvcs_.find(svcName);

    if (it == this->mockSvcs_.end()) {
        return NULL;
    } else {
        apis = it->second;
    }

    MockApiCall::iterator ait = apis->find(apiName);
    if (ait == apis->end()) {
        return NULL;
    } else {
        ApiResponses& responses = ait->second;
        size_t idx = responses.idx++;

        if (idx < responses.queue.size()) {
            return responses.queue.at(idx);
        } else {
            return NULL;
        }
    }
}

bool UTModule::pushServiceCall(const std::string& svcName,
                               const std::string& apiName,
                               const google::protobuf::Message& response)
{
    // 1. Find or create a mock service
    MockApiCall* apis = NULL;
    MockSvcs::iterator it = this->mockSvcs_.find(svcName);

    if (it == this->mockSvcs_.end()) {
        skullmock_svc_t mockSvc;
        mockSvc.name = svcName.c_str();
        mockSvc.ud   = this;
        mockSvc.iocall  = _iocall;
        mockSvc.release = _release;
        skullut_module_mocksrv_add(this->utModule_, mockSvc);

        apis = new MockApiCall();
        this->mockSvcs_.insert(std::make_pair(svcName, apis));
    } else {
        apis = it->second;
    }

    // 2. Find or create a api
    MockApiCall::iterator ait = apis->find(apiName);
    if (ait != apis->end()) {
        ApiResponses& responses = ait->second;
        responses.queue.push_back(&response);
    } else {
        ApiResponses responses;
        responses.queue.push_back(&response);

        apis->insert(std::make_pair(apiName, responses));
    }

    return true;
}

void UTModule::setTxnSharedData(const google::protobuf::Message& data) {
    // 1. prepare rawData
    TxnSharedRawData* rawData =
        (TxnSharedRawData*)skullut_module_data(this->utModule_);

    if (!rawData) {
        rawData = new TxnSharedRawData();
    }

    // 2. serialize msg to rawData
    void* idlData = NULL;
    int sz = data.ByteSize();
    if (sz) {
        idlData = calloc(1, (size_t)sz);
        bool r = data.SerializeToArray(idlData, sz);
        assert(r);
    }

    rawData->reset(idlData, (size_t)sz);

    // 3. store rawData
    skullut_module_setdata(this->utModule_, rawData);
}

// The detail implementation is refer to txn.cpp
google::protobuf::Message& UTModule::getTxnSharedData() {
    if (this->msg_) {
        delete this->msg_;
        this->msg_ = NULL;
    }

    const std::string& idlName = this->idlName_;
    std::string protoFileName = idlName + ".proto";

    const FileDescriptor* fileDesc =
        DescriptorPool::generated_pool()->FindFileByName(protoFileName);
    const Descriptor* desc = fileDesc->message_type(0);

    const Message* new_msg =
        MessageFactory::generated_factory()->GetPrototype(desc);
    this->msg_ = new_msg->New();

    TxnSharedRawData* rawData =
        (TxnSharedRawData*)skullut_module_data(this->utModule_);

    if (rawData) {
        const void* binData = rawData->data();
        if (binData) {
            int size = (int)rawData->size();
            bool r = this->msg_->ParseFromArray(binData, size);
            assert(r);
        }
    }

    return *this->msg_;
}

const google::protobuf::Message& UTModule::getTxnSharedData() const {
    return getTxnSharedData();
}

/****************************** Service APIs **********************************/
UTService::UTService(const std::string& svcName, const std::string& config) {
    this->svcName_   = svcName;
    this->utService_ = skullut_service_create(svcName.c_str(), config.c_str(),
                                              "cpp", svc_getloader());
}

UTService::~UTService() {
    skullut_service_destroy(this->utService_);
}

class ServiceAPIData {
public:
    UTService& service_;
    const std::string& apiName_;
    google::protobuf::Message& uResp_;

public:
    ServiceAPIData(UTService& svc, const std::string& apiName,
        google::protobuf::Message& uResp)
        : service_(svc), apiName_(apiName), uResp_(uResp) {}

    ~ServiceAPIData() {}
};

static
void _api_validator(const void* req, size_t req_sz,
                    const void* resp, size_t resp_sz, void* ud) {
    ServiceAPIData* apiData = (ServiceAPIData*)ud;
    ServiceApiRespData apiResp(apiData->service_.svcName().c_str(),
                               apiData->apiName_.c_str(),
                               resp, resp_sz);

    apiData->uResp_.CopyFrom(apiResp.get());
}

void UTService::run(const std::string& apiName,
                    const google::protobuf::Message& req,
                    google::protobuf::Message& resp) {
    // 1. construct a service request
    ServiceApiReqData apiReq(this->svcName_.c_str(), apiName.c_str(), req, NULL);
    size_t dataSz = 0;
    ServiceApiReqRawData* rawReqData = apiReq.serialize(dataSz);

    // 2. construct api data
    ServiceAPIData apiData(*this, apiName, resp);

    // 3. run service api
    skullut_service_run(this->utService_, apiName.c_str(), rawReqData, dataSz,
                        _api_validator, &apiData);
}

const std::string& UTService::svcName() {
    return this->svcName_;
}

} // End of namespace