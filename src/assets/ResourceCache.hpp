#pragma once

#include "AssetId.hpp"

#include <memory>
#include <unordered_map>

namespace aegis::assets {

template <typename Id, typename Resource>
class ResourceCache {
public:
    ResourceCache() = default;
    explicit ResourceCache(Resource& fallback) : fallback_(&fallback) {}
    void setFallback(Resource& fallback) { fallback_ = &fallback; }

    template <typename Loader>
    Resource& get(const Id& id, Loader&& loader) {
        if (const auto found = resources_.find(id); found != resources_.end()) return *found->second;
        ++requests_[id];
        auto resource = loader();
        if (!resource) return *fallback_;
        auto& result = *resource;
        resources_.emplace(id, std::move(resource));
        return result;
    }

    std::size_t requestCount(const Id& id) const {
        const auto found = requests_.find(id);
        return found == requests_.end() ? 0 : found->second;
    }
    std::size_t size() const { return resources_.size(); }

private:
    Resource* fallback_ = nullptr;
    std::unordered_map<Id, std::unique_ptr<Resource>, AssetIdHash> resources_;
    std::unordered_map<Id, std::size_t, AssetIdHash> requests_;
};

} // namespace aegis::assets
