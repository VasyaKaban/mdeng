#pragma once

#include "../instantiation.hpp"
#include <concepts>

namespace hrs
{
    struct map_unique_key
    {};
    struct map_shared_key
    {};

    template<typename K, typename M, typename C>
    requires std::same_as<M, map_unique_key> || std::same_as<M, map_shared_key>
    struct key
    {
        using key_t = K;
        using key_map_t = M;
        using comparator_t = C;
    };

    template<typename V, typename A, type_instantiation<key> CKey, type_instantiation<key>... CKeys>
    class basic_multikey_map;
};
