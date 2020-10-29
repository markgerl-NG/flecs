#ifndef FLECS_PAGED_H
#define FLECS_PAGED_H

#include "api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecs_paged_t ecs_paged_t;

FLECS_EXPORT
ecs_paged_t* _ecs_paged_new(
    ecs_size_t elem_size);

#define ecs_paged_new(T)\
    _ecs_paged_new(ECS_SIZEOF(T))

FLECS_EXPORT
void ecs_paged_free(
    ecs_paged_t *paged);

FLECS_EXPORT
void _ecs_paged_set(
    ecs_paged_t *paged,
    ecs_size_t elem_size,
    uint64_t index,
    void *value);

#define ecs_paged_set(paged, T, index, value)\
    _ecs_paged_set(paged, ECS_SIZEOF(T), index, value)

FLECS_EXPORT
void* _ecs_paged_get(
    ecs_paged_t *paged,
    ecs_size_t elem_size,
    uint64_t index);

#define ecs_paged_get(paged, T, index)\
    (T*)_ecs_paged_get(paged, ECS_SIZEOF(T), index)

#ifdef __cplusplus
}
#endif

#endif
