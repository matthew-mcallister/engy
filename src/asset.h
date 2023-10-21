#ifndef ASSET_H_INCLUDED
#define ASSET_H_INCLUDED

#include <fstream>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "exceptions.h"

typedef char byte;

struct AssetResolver {
    virtual std::vector<byte> resolve(const std::string &path) = 0;
};

/// Loads files off of disk based on file path, uncached.
class DirectoryAssetResolver : public AssetResolver {
    std::string m_root;

public:
    DirectoryAssetResolver(std::string &&root);
    virtual std::vector<byte> resolve(const std::string &path);
};

class AssetApi {
    std::unique_ptr<AssetResolver> m_resolver;

public:
    AssetApi(std::unique_ptr<AssetResolver> &&resolver);

    std::vector<byte> load_blob(const std::string &path);
    std::string load_text(const std::string &path);
};

#endif