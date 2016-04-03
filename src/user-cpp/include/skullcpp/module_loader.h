#ifndef SKULLCPP_MOCULE_LOADER_H
#define SKULLCPP_MOCULE_LOADER_H

#include <skull/config.h>
#include <skullcpp/txn.h>
#include <skullcpp/txndata.h>

namespace skullcpp {

typedef struct ModuleEntry {
    void   (*init)    (const skull_config_t*);
    void   (*release) ();

    int    (*run)     (Txn&);
    size_t (*unpack)  (Txn&, const void* data, size_t data_sz);
    void   (*pack)    (Txn&, TxnData&);
} ModuleEntry;

} // End of namespace

#define SKULLCPP_MODULE_REGISTER(entry) \
    extern "C" { \
        skullcpp::ModuleEntry* skullcpp_module_register() { \
            return entry; \
        } \
    }

#endif
