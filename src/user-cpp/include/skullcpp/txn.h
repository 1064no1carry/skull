#ifndef SKULLCPP_TXN_H
#define SKULLCPP_TXN_H

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <google/protobuf/message.h>

#include <skull/txn.h>

namespace skullcpp {

class Txn {
private:
    // Make noncopyable
    Txn(const Txn&) = delete;
    Txn(Txn&&) = delete;
    Txn& operator=(const Txn&) = delete;
    Txn& operator=(Txn&&) = delete;

public:
    typedef enum Status {
        TXN_OK = 0,
        TXN_ERROR
    } Status;

    typedef enum IOStatus {
        OK            = 0,
        ERROR_SRVNAME = 1,
        ERROR_APINAME = 2,
        ERROR_BIO     = 3
    } IOStatus;

    typedef int (*ApiCB) (Txn&, const std::string& apiName,
                          const google::protobuf::Message& request,
                          const google::protobuf::Message& response);

public:
    Txn() {};
    virtual ~Txn() {};

public:
    virtual google::protobuf::Message& data() = 0;
    virtual Status status() = 0;

    /**
     * Invoke a service async call
     *
     * @param serivce_name
     * @param apiName
     * @param request       request protobuf message
     * @param cb            api callback function
     *
     * @return - OK
     *         - ERROR_SRVNAME
     *         - ERROR_APINAME
     */
    virtual IOStatus serviceCall (const std::string& serviceName,
                          const std::string& apiName,
                          const google::protobuf::Message& request,
                          ApiCB cb) = 0;

    /**
     * Invoke a service async call
     *
     * @param serivce_name
     * @param apiName
     * @param request       request protobuf message
     * @param bio_idx       background io index
     *                      - (-1)  : random pick up a background io to run
     *                      - (0)   : do not use background io
     *                      - (> 0) : run on the index of background io
     * @param cb            api callback function
     *
     * @return - OK
     *         - ERROR_SRVNAME
     *         - ERROR_APINAME
     *         - ERROR_BIO
     */
    virtual IOStatus serviceCall (const std::string& serviceName,
                          const std::string& apiName,
                          const google::protobuf::Message& request,
                          int bio_idx,
                          ApiCB cb) = 0;
};

} // End of namespace

#endif

