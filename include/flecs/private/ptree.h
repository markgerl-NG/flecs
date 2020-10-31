#ifndef FLECS_PTREE_H
#define FLECS_PTREE_H

#include "api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecs_ptree_t ecs_ptree_t;

FLECS_API ecs_ptree_t* _ecs_ptree_new(
    ecs_size_t elem_size);

FLECS_API uint32_t ecs_ptree_free(
    ecs_ptree_t *ptree);

FLECS_API void* _ecs_ptree_ensure(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index);

FLECS_API void* _ecs_ptree_get(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index);

#define ecs_ptree_new(T)\
    _ecs_ptree_new(ECS_SIZEOF(T))

#define ecs_ptree_get(ptree, T, index)\
    (T*)_ecs_ptree_get(ptree, ECS_SIZEOF(T), index)

#define ecs_ptree_ensure(ptree, T, index)\
    _ecs_ptree_ensure(ptree, ECS_SIZEOF(T), index)


/* -- Low footprint version, do not mix API calls -- */

FLECS_API ecs_ptree_t* _ecs_ptiny_new(
    ecs_size_t elem_size);

FLECS_API void* _ecs_ptiny_ensure(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index);

FLECS_API void* _ecs_ptiny_get(
    ecs_ptree_t *ptree,
    ecs_size_t elem_size,
    uint64_t index);

#define ecs_ptiny_new(T)\
    _ecs_ptiny_new(ECS_SIZEOF(T))

#define ecs_ptiny_get(ptree, T, index)\
    (T*)_ecs_ptiny_get(ptree, ECS_SIZEOF(T), index)

#define ecs_ptiny_ensure(ptree, T, index)\
    _ecs_ptiny_ensure(ptree, ECS_SIZEOF(T), index)

#define ecs_ptiny_free(ptree)\
    ecs_ptree_free(ptree)

#ifdef __cplusplus
}
#endif

#endif
