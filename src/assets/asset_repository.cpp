#include <assets/asset_repository.hpp>

Asset get_asset(name owner, uint64_t asset_id) {
    Asset result;
    aa::assets_t assets = aa::get_assets(owner);
    result.owner = owner;
    result.asset = assets.get(asset_id, ("[get_asset]: Wrong asset id = " + to_string(asset_id)).c_str());

    check(result.asset.collection_name == COLLECTION, "Asset is taken from the wrong collection " + result.asset.collection_name.to_string());

    aa::schemas_t schemas = aa::get_schemas(result.asset.collection_name);
    result.schemas = schemas.get(result.asset.schema_name.value,    "There is no asset's schema.");

    aa::templates_t templates = aa::get_templates(result.asset.collection_name);
    check(result.asset.schema_name == FOODS || result.asset.schema_name == ANIMALS, "Wrong schema name for asset.");
    result.templates = templates.get(result.asset.template_id,("There is no template with id " + to_string(result.asset.template_id)).c_str());

    return result;
}
