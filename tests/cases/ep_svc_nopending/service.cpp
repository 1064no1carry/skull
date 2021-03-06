#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <iostream>
#include <google/protobuf/message.h>

#include <skullcpp/api.h>
#include "skull_protos.h"
#include "config.h"

// ====================== Service Init/Release =================================
static
void skull_service_init(skullcpp::Service& service, const skull_config_t* config)
{
    printf("skull service init\n");
    skullcpp::Config::instance().load(config);
}

static
void skull_service_release(skullcpp::Service& service)
{
    printf("skull service release\n");
}

static
ssize_t _ep_unpack(const void* data, size_t len)
{
    std::cout << "ep received data len: " << len << std::endl;
    return (ssize_t)len;
}

static
void _ep_release(void* ud)
{
    std::cout << "ep released" << std::endl;
}

static
void _ep_cb(const skullcpp::Service&, skullcpp::EPClientNPRet& ret)
{
    std::cout << "in _ep_cb: status: " << ret.status() << ", data len: " << ret.responseSize() << std::endl;
    SKULL_LOG_INFO("svc.test.ep_cb", "status: %d, responseSize: %zu, latency: %d",
                   ret.status(), ret.responseSize(), ret.latency());

    SKULLCPP_LOG_INFO("svc.test.ep_cb1", "ip: " << ret.ip() << ", "
                      << "port: "    << ret.port()    << ", "
                      << "timeout: " << ret.timeout() << ", "
                      << "type: "    << ret.type()    << ", "
                      << "flags: "   << ret.flags());

    if (ret.status() == skullcpp::EPClient::OK) {
        SKULLCPP_LOG_INFO("svc.test.ep_cb-1", "ep response: " << std::string((char*)ret.response()));
    } else {
        SKULLCPP_LOG_INFO("svc.test.ep_cb-2", "ep timeout or error");
    }
}

// ====================== Service APIs Calls ===================================
static
void skull_service_getdata(const skullcpp::Service& service,
                           const google::protobuf::Message& request,
                           google::protobuf::Message& response)
{
    printf("skull service api: getdata\n");
    SKULL_LOG_INFO("svc.test.get-1", "service get data");

    const auto& apiReq = (skull::service::s1::get_req&)request;
    auto& apiResp = (skull::service::s1::get_resp&)response;

    std::cout << "api req: " << apiReq.name() << std::endl;
    apiResp.set_response("Hi new bie");

    skullcpp::EPClient epClient;
    epClient.setType(skullcpp::EPClient::TCP);
    epClient.setIP("127.0.0.1");
    epClient.setPort(7760);
    epClient.setTimeout(3000);
    epClient.setUnpack(skull_BindEpUnpack(_ep_unpack));

    skullcpp::EPClient::Status st =
        epClient.send(service, "hello ep", skull_BindEpNPCb(_ep_cb));
    std::cout << "ep status: " << st << std::endl;
    SKULL_LOG_INFO("svc.test-get-2", "ep status: %d", st);
}

// ====================== Register Service =====================================
static skullcpp::ServiceReadApi api_read_tbl[] = {
    {"get", skull_service_getdata},
    {NULL, NULL}
};

static skullcpp::ServiceEntry service_entry = {
    skull_service_init,
    skull_service_release,
    api_read_tbl,
    NULL
};

SKULLCPP_SERVICE_REGISTER(&service_entry)
