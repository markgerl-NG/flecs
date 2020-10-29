#include "flecs.h"

typedef struct addr_t {
    uint8_t value[8];
} addr_t;

typedef struct array_t {
    void *data;
    uint8_t offset;
    uint16_t length;
} array_t;

typedef struct page_t {
    array_t data;
    array_t pages;
} page_t;

struct ecs_paged_t {
    page_t root;
};

static
addr_t to_addr(
    uint64_t id) 
{
    union {
        addr_t addr;
        uint64_t id;
    } u = {.id = id};
    return u.addr;
}

static
void* array_ensure(
    array_t *array,
    ecs_size_t elem_size,
    uint8_t index)
{
    uint8_t offset = array->offset;
    uint16_t length = array->length;

    if (!length) {
        array->length = 1;
        array->offset = index;
        array->data = ecs_os_calloc(elem_size);
        ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);
        return array->data; 
    } else if (index < offset) {
        uint16_t diff = offset - index;
        uint16_t new_length = length + diff;
        array->data = ecs_os_realloc(array->data, new_length * elem_size);
        ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);
        memmove(ECS_OFFSET(array->data, diff * elem_size), array->data, 
            length * elem_size);
        memset(array->data, 0, diff * elem_size);
        array->offset = index;
        array->length = new_length;
        ecs_assert((array->offset + array->length) <= 256, 
            ECS_INTERNAL_ERROR, NULL);
        return array->data;
    } else if (index >= (offset + length)) {
        uint16_t diff = index - (offset + length - 1);
        uint16_t new_length = length + diff;
        array->length = new_length;
        array->data = ecs_os_realloc(array->data, new_length * elem_size);
        ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);
        memset(ECS_OFFSET(array->data, length * elem_size), 0, 
            diff * elem_size);
        ecs_assert((array->offset + array->length) <= 256, 
            ECS_INTERNAL_ERROR, NULL);
    }

    return ECS_OFFSET(array->data, (index - offset) * elem_size);
}

static
void* array_get(
    array_t *array,
    ecs_size_t elem_size,
    uint16_t index)
{
    uint8_t offset = array->offset;
    uint16_t length = array->length;

    if (index < offset) {
        return NULL;
    }

    index -= offset;
    if (index >= length) {
        return NULL;
    }

    return ECS_OFFSET(array->data, index * elem_size);
}

static
page_t* get_page(
    page_t *p,
    uint8_t *addr,
    int count) 
{
    int32_t i;
    for (i = count - 1; i > 0; i --) {
        p = array_get(&p->pages, ECS_SIZEOF(page_t), addr[i]);
        if (!p) {
            return NULL;
        }
    }

    return p;
}

static
page_t* get_or_create_page(
    page_t *p,
    uint8_t *addr,
    int count)
{
    int32_t i;
    for (i = count - 1; i > 0; i --) {
        uint8_t index = addr[i];
        p = array_ensure(&p->pages, ECS_SIZEOF(page_t), index);
        ecs_assert(p != NULL, ECS_INTERNAL_ERROR, NULL);
    }

    return p;
}

static
int8_t page_count(
    uint64_t index)
{
    return 1 +
        (index > 0x00000000000000FF) +
        (index > 0x000000000000FF00) +
        (index > 0x0000000000FF0000) +
        (index > 0x00000000FF000000) +
        (index > 0xFFFFFFFF00000000) * 3;
}

static
void page_free(
    page_t *p)
{
    if (p->data.data) {
        ecs_os_free(p->data.data);
    }

    if (p->pages.data) {
        uint16_t i;
        for (i = 0; i < p->pages.length; i ++) {
            page_free(array_get(&p->pages, ECS_SIZEOF(page_t), 
                i + p->pages.offset));
        }

        ecs_os_free(p->pages.data);
    }
}

ecs_paged_t* _ecs_paged_new(
    ecs_size_t elem_size)
{
    (void)elem_size;
    ecs_paged_t *result = ecs_os_calloc(sizeof(ecs_paged_t));
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY, NULL);
    return result;
}

void ecs_paged_free(
    ecs_paged_t *paged)
{
    page_free(&paged->root);
    ecs_os_free(paged);
}

void _ecs_paged_set(
    ecs_paged_t *paged,
    ecs_size_t elem_size,
    uint64_t index,
    void *value)
{
    addr_t addr = to_addr(index);
    page_t *p = get_or_create_page(&paged->root, addr.value, page_count(index));
    ecs_assert(p != NULL, ECS_INTERNAL_ERROR, NULL);
    void *data = array_ensure(&p->data, elem_size, addr.value[0]);
    ecs_assert(data != NULL, ECS_INTERNAL_ERROR, NULL);
    ecs_os_memcpy(data, value, elem_size);
}

void* _ecs_paged_get(
    ecs_paged_t *paged,
    ecs_size_t elem_size,
    uint64_t index)
{
    addr_t addr = to_addr(index);
    page_t *p = get_page(&paged->root, addr.value, page_count(index));
    if (!p) {
        return NULL;
    }
    return array_get(&p->data, elem_size, addr.value[0]);
}
