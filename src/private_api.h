#ifndef FLECS_PRIVATE_H
#define FLECS_PRIVATE_H

#include "private_types.h"


////////////////////////////////////////////////////////////////////////////////
//// Core bootstrap functions
////////////////////////////////////////////////////////////////////////////////

#define ECS_TYPE_DECL(component)\
static const ecs_entity_t __##component = ecs_typeid(component);\
ECS_VECTOR_DECL(FLECS__T##component, ecs_entity_t, 1)

#define ECS_TYPE_IMPL(component)\
ECS_VECTOR_IMPL(FLECS__T##component, ecs_entity_t, &__##component, 1)

/* Bootstrap world */
void ecs_bootstrap(
    ecs_world_t *world);

ecs_type_t ecs_bootstrap_type(
    ecs_world_t *world,
    ecs_entity_t entity);

#define ecs_bootstrap_component(world, name)\
    ecs_new_component(world, ecs_typeid(name), #name, sizeof(name), ECS_ALIGNOF(name))

#define ecs_bootstrap_tag(world, name)\
    ecs_set(world, name, EcsName, {.value = &#name[ecs_os_strlen("Ecs")], .symbol = #name});\
    ecs_add_id(world, name, ecs_role(EcsScope, ecs_get_scope(world)))


////////////////////////////////////////////////////////////////////////////////
//// Entity API
////////////////////////////////////////////////////////////////////////////////

/* Mark an entity as being watched. This is used to trigger automatic rematching
 * when entities used in system expressions change their components. */
void ecs_set_watch(
    ecs_world_t *world,
    ecs_entity_t entity);

/* Does one of the entity containers has specified component */
ecs_entity_t ecs_find_in_type(
    ecs_world_t *world,
    ecs_type_t table_type,
    ecs_entity_t component,
    ecs_entity_t flags);

/* Obtain entity info */
bool ecs_get_info(
    ecs_world_t *world,
    ecs_entity_t entity,
    ecs_entity_info_t *info);

void ecs_run_monitors(
    ecs_world_t *world, 
    ecs_table_t *dst_table,
    ecs_vector_t *v_dst_monitors, 
    int32_t dst_row, 
    int32_t count, 
    ecs_vector_t *v_src_monitors);

const char* ecs_role_str(
    ecs_entity_t entity);

size_t ecs_entity_str(
    ecs_world_t *world,
    ecs_entity_t entity,
    char *buffer,
    size_t buffer_len);


////////////////////////////////////////////////////////////////////////////////
//// World API
////////////////////////////////////////////////////////////////////////////////

/* Notify systems that there is a new table, which triggers matching */
void ecs_notify_queries_of_table(
    ecs_world_t *world,
    ecs_table_t *table);

/* Get current thread-specific stage */
ecs_stage_t* ecs_get_stage(
    ecs_world_t **world_ptr);

/* Get component callbacks */
ecs_lifecycle_t* ecs_get_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component);

/* Get or create component callbacks */
ecs_lifecycle_t* ecs_get_or_create_lifecycle(
    ecs_world_t *world,
    ecs_entity_t component);

bool ecs_defer_op_begin(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_defer_kind_t op_kind,
    ecs_entity_t entity,
    ecs_entities_t *components,
    const void *value,
    ecs_size_t size);

void ecs_defer_flush(
    ecs_world_t *world,
    ecs_stage_t *stage);
    

////////////////////////////////////////////////////////////////////////////////
//// Stage API
////////////////////////////////////////////////////////////////////////////////

/* Initialize stage data structures */
void ecs_stage_init(
    ecs_world_t *world,
    ecs_stage_t *stage);

/* Deinitialize stage */
void ecs_stage_deinit(
    ecs_world_t *world,
    ecs_stage_t *stage);

/* Merge stage with main stage */
void ecs_stage_merge(
    ecs_world_t *world,
    ecs_stage_t *stage);

/* Begin defer for stage */
void ecs_stage_defer_begin(
    ecs_world_t *world,
    ecs_stage_t *stage);

void ecs_stage_defer_end(
    ecs_world_t *world,
    ecs_stage_t *stage);    

/* Delete table from stage */
void ecs_delete_table(
    ecs_world_t *world,
    ecs_table_t *table);


////////////////////////////////////////////////////////////////////////////////
//// Defer API
////////////////////////////////////////////////////////////////////////////////

bool ecs_defer_none(
    ecs_world_t *world,
    ecs_stage_t *stage);

bool ecs_defer_modified(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component);

bool ecs_defer_new(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entities_t *components);

bool ecs_defer_delete(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity);

bool ecs_defer_clear(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity);

bool ecs_defer_add(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component);

bool ecs_defer_remove(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_entity_t entity,
    ecs_entity_t component);

bool ecs_defer_set(
    ecs_world_t *world,
    ecs_stage_t *stage,
    ecs_defer_kind_t op_kind,
    ecs_entity_t entity,
    ecs_entity_t component,
    ecs_size_t size,
    const void *value,
    void **value_out,
    bool *is_added);

void ecs_defer_flush(
    ecs_world_t *world,
    ecs_stage_t *stage);


////////////////////////////////////////////////////////////////////////////////
//// Table API
////////////////////////////////////////////////////////////////////////////////

/** Find or create table for a set of components */
ecs_table_t* ecs_table_find_or_create(
    ecs_world_t *world,
    ecs_entities_t *type);

/** Find or create table for a type */
ecs_table_t* ecs_table_from_type(
    ecs_world_t *world,
    ecs_type_t type);    

/* Get table data */
ecs_data_t *ecs_table_get_data(
    ecs_table_t *table);

/* Get or create data */
ecs_data_t *ecs_table_get_or_create_data(
    ecs_table_t *table);

/* Initialize columns for data */
ecs_data_t* ecs_init_data(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *result); 

/* Clear all entities from a table. */
void ecs_table_clear(
    ecs_world_t *world,
    ecs_table_t *table);

/* Reset a table to its initial state */
void ecs_table_reset(
    ecs_world_t *world,
    ecs_table_t *table);

/* Clear all entities from the table. Do not invoke OnRemove systems */
void ecs_table_clear_silent(
    ecs_world_t *world,
    ecs_table_t *table);

/* Clear table data. Don't call OnRemove handlers. */
void ecs_table_clear_data(
    ecs_table_t *table,
    ecs_data_t *data);    

/* Return number of entities in table. */
int32_t ecs_table_count(
    ecs_table_t *table);

/* Return number of entities in data */
int32_t ecs_table_data_count(
    ecs_data_t *data);

/* Add a new entry to the table for the specified entity */
int32_t ecs_table_append(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    ecs_entity_t entity,
    ecs_record_t *record,
    bool construct);

/* Delete an entity from the table. */
void ecs_table_delete(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    int32_t index,
    bool destruct);

/* Move a row from one table to another */
void ecs_table_move(
    ecs_world_t *world,
    ecs_entity_t dst_entity,
    ecs_entity_t src_entity,
    ecs_table_t *new_table,
    ecs_data_t *new_data,
    int32_t new_index,
    ecs_table_t *old_table,
    ecs_data_t *old_data,
    int32_t old_index);

/* Grow table with specified number of records. Populate table with entities,
 * starting from specified entity id. */
int32_t ecs_table_appendn(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    int32_t count,
    const ecs_entity_t *ids);

/* Set table to a fixed size. Useful for preallocating memory in advance. */
void ecs_table_set_size(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_data_t *data,
    int32_t count);

/* Get dirty state for table columns */
int32_t* ecs_table_get_dirty_state(
    ecs_table_t *table);

/* Initialize root table */
void ecs_init_root_table(
    ecs_world_t *world,
    ecs_table_t *root);

/* Free table */
void ecs_table_free(
    ecs_world_t *world,
    ecs_table_t *table); 

ecs_table_t *ecs_table_traverse_add(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t to_add);

ecs_table_t *ecs_table_traverse_remove(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t to_remove);

void ecs_table_mark_dirty(
    ecs_table_t *table,
    ecs_entity_t component);

ecs_entity_t ecs_component_id_from_id(
    ecs_world_t *world,
    ecs_entity_t e);

const EcsComponent* ecs_component_from_id(
    ecs_world_t *world,
    ecs_entity_t e);

int32_t ecs_table_switch_from_case(
    ecs_world_t *world,
    ecs_table_t *table,
    ecs_entity_t add);    

void ecs_table_clear_edges(
    ecs_table_t *table);

void ecs_table_notify(
    ecs_world_t * world,
    ecs_table_t * table,
    ecs_table_event_t * event);


////////////////////////////////////////////////////////////////////////////////
//// Time API
////////////////////////////////////////////////////////////////////////////////

void ecs_os_time_setup(void);

uint64_t ecs_os_time_now(void);

void ecs_os_time_sleep(
    int32_t sec, 
    int32_t nanosec);

/* Increase or reset timer resolution (Windows only) */
FLECS_API
void ecs_increase_timer_resolution(
    bool enable);


////////////////////////////////////////////////////////////////////////////////
//// Utilities
////////////////////////////////////////////////////////////////////////////////

void ecs_hash(
    const void *data,
    size_t length,
    uint64_t *result);

/* Convert 64 bit signed integer to 16 bit */
int8_t ecs_to_i8(
    int64_t v);

/* Convert 64 bit signed integer to 16 bit */
int16_t ecs_to_i16(
    int64_t v);

/* Convert 64 bit unsigned integer to 32 bit */
uint32_t ecs_to_u32(
    uint64_t v);        

/* Convert signed integer to size_t */
size_t ecs_to_size_t(
    int64_t size);

/* Convert size_t to ecs_size_t */
ecs_size_t ecs_from_size_t(
    size_t size);    

/* Convert 64bit value to ecs_record_t type. ecs_record_t is stored as 64bit int in the
 * entity index */
ecs_record_t ecs_to_row(
    uint64_t value);

/* Get 64bit integer from ecs_record_t */
uint64_t ecs_from_row(
    ecs_record_t record);

/* Get actual row from record row */
int32_t ecs_record_to_row(
    int32_t row, 
    bool *is_watched_out);

/* Convert actual row to record row */
int32_t ecs_row_to_record(
    int32_t row, 
    bool is_watched);

/* Convert type to entity array */
ecs_entities_t ecs_type_to_entities(
    ecs_type_t type); 

/* Convert a symbol name to an entity name by removing the prefix */
const char* ecs_name_from_symbol(
    ecs_world_t *world,
    const char *type_name); 

/* Lookup an entity by name with a specific id */
ecs_entity_t ecs_lookup_w_id(
    ecs_world_t *world,
    ecs_entity_t e,
    const char *name);

/* Set entity name with symbol */
void ecs_set_symbol(
    ecs_world_t *world,
    ecs_entity_t e,
    const char *name);

#define assert_func(cond) _assert_func(cond, #cond, __FILE__, __LINE__, __func__)
void _assert_func(
    bool cond,
    const char *cond_str,
    const char *file,
    int32_t line,
    const char *func);

#endif
