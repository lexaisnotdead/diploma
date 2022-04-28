#pragma once
#include <atomicassets/atomicassets.hpp>
#include <atomicassets/read.hpp>
#include <diplomadef.hpp>

class Asset {
public:
    name owner{};
    aa::assets_s asset;
    aa::schemas_s schemas;
    aa::templates_s templates;
};