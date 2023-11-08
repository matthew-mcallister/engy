#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <concepts>
#include <cstdint>

#include <vulkan/vulkan.hpp>

template<typename T>
struct object_type {};

#define DECLARE_OBJECT_TYPE(_T_)                                               \
    template<>                                                                 \
    struct object_type<vk::raii::_T_> {                                        \
        static const vk::ObjectType value = vk::ObjectType::e##_T_;            \
    };                                                                         \
    template<>                                                                 \
    struct object_type<vk::_T_> {                                              \
        static const vk::ObjectType value = vk::ObjectType::e##_T_;            \
    };

DECLARE_OBJECT_TYPE(Buffer);
DECLARE_OBJECT_TYPE(CommandPool);
DECLARE_OBJECT_TYPE(CommandBuffer);
DECLARE_OBJECT_TYPE(Semaphore);

#endif
