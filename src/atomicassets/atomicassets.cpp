#include <atomicassets/atomicassets.hpp>
#include <eosio/eosio.hpp>

using namespace eosio;
using namespace std;

namespace aa {
    collections_t  collections  = collections_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    offers_t       offers       = offers_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    balances_t     balances     = balances_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    config_t       config       = config_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
    tokenconfigs_t tokenconfigs = tokenconfigs_t(ATOMICASSETS_ACCOUNT, ATOMICASSETS_ACCOUNT.value);
};
