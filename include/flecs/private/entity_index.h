#ifndef FLECS_ENTITY_INDEX_H
#define FLECS_ENTITY_INDEX_H

#include "api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ecs_eis_get(world, entity) ecs_sparse_get_sparse((world->store).entity_index, ecs_record_t, entity)
#define ecs_eis_get_any(world, entity) ecs_sparse_get_sparse_any((world->store).entity_index, ecs_record_t, entity)
#define ecs_eis_set(world, entity, ...) (ecs_sparse_set((world->store).entity_index, ecs_record_t, entity, (__VA_ARGS__)))
#define ecs_eis_get_or_create(world, entity) ecs_sparse_get_or_create((world->store).entity_index, ecs_record_t, entity)
#define ecs_eis_delete(world, entity) ecs_sparse_remove((world->store).entity_index, entity)
#define ecs_eis_set_generation(world, entity) ecs_sparse_set_generation((world->store).entity_index, entity)
#define ecs_eis_is_alive(world, entity) ecs_sparse_is_alive((world->store).entity_index, entity)
#define ecs_eis_exists(world, entity) ecs_sparse_exists((world->store).entity_index, entity)
#define ecs_eis_recycle(world) ecs_sparse_new_id((world->store).entity_index)
#define ecs_eis_clear_entity(world, entity, is_watched) ecs_eis_set((world->store).entity_index, entity, &(ecs_record_t){NULL, is_watched})
#define ecs_eis_set_size(world, size) ecs_sparse_set_size((world->store).entity_index, size)
#define ecs_eis_count(world) ecs_sparse_count((world->store).entity_index)
#define ecs_eis_clear(world) ecs_sparse_clear((world->store).entity_index)
#define ecs_eis_copy(world) ecs_sparse_copy((world->store).entity_index)
#define ecs_eis_free(world) ecs_sparse_free((world->store).entity_index)
#define ecs_eis_memory(world, allocd, used) ecs_sparse_memory((world->store).entity_index, allocd, used)

#ifdef __cplusplus
}
#endif

#endif
