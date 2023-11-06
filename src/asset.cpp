#include <cstdlib>
#include <format>
#include <fstream>
#include <vector>

#include "asset.h"

DirectoryAssetResolver::DirectoryAssetResolver(std::string &&root)
    : m_root(std::move(root)) {}

std::vector<char> DirectoryAssetResolver::resolve(const std::string &path) {
    std::string real_path;
    if (path[0] == '/') {
        real_path = m_root + path;
    } else {
        real_path = m_root + '/' + path;
    }

    std::ifstream f(real_path, std::ios_base::binary);
    if (!f.good()) {
        throw SystemException(std::format("Cannot read file: {}", real_path));
    }
    return std::vector(std::istreambuf_iterator{f}, {});
}

AssetApi::AssetApi(std::unique_ptr<AssetResolver> &&resolver)
    : m_resolver(std::move(resolver)) {}

std::vector<byte> AssetApi::load_blob(const std::string &path) {
    return m_resolver->resolve(path);
}

std::string AssetApi::load_text(const std::string &path) {
    auto bytes = m_resolver->resolve(path);
    return std::string(bytes.begin(), bytes.end());
}
