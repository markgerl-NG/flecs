#include "flecs.h"

#define PAGE_SIZE (65536)

typedef struct addr_t {
    uint16_t value[4];
} addr_t;

typedef struct array_t {
    void *data;
    uint16_t offset;
    uint32_t length;
} array_t;

typedef struct page_t {
    array_t data;
    array_t pages;
} page_t;

struct ecs_paged_t {
    page_t root;
    void *first_65k;
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
int32_t next_pow_of_2(
    int32_t n)
{
    n --;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n ++;

    return n;
}

static
void* array_ensure(
    array_t *array,
    ecs_size_t elem_size,
    uint16_t index)
{
    uint16_t offset = array->offset;
    uint32_t length = array->length;

    if (!length) {
        array->length = 1;
        array->offset = index;
        array->data = ecs_os_calloc(elem_size);
        ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);
        return array->data; 

    } else if (index < offset) {
        uint32_t new_offset = next_pow_of_2(index) >> 1;
        uint32_t dif_offset = offset - new_offset;
        uint32_t new_length = next_pow_of_2(length + dif_offset);
        
        if ((new_offset + new_length) > PAGE_SIZE) {
            new_offset = 0;
            new_length = PAGE_SIZE;
            dif_offset = offset;
        }

        ecs_assert(length != new_length, ECS_INTERNAL_ERROR, NULL);
        array->data = ecs_os_realloc(array->data, new_length * elem_size);
        ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);

        memset(ECS_OFFSET(array->data, length * elem_size), 0, (new_length - length) * elem_size);
        memmove(ECS_OFFSET(array->data, dif_offset * elem_size), array->data, 
            length * elem_size);
        memset(array->data, 0, dif_offset * elem_size);            
        array->offset = new_offset;
        array->length = new_length;

    } else if (index >= (offset + length)) {
        uint32_t new_length = next_pow_of_2(index + offset + 1);
        if ((new_length + offset) > PAGE_SIZE) {
            uint32_t new_offset = next_pow_of_2(offset - ((new_length + offset) - PAGE_SIZE)) >> 1;
            uint32_t dif_offset = offset - new_offset;
            new_length = next_pow_of_2(new_offset + index);
            
            ecs_assert(length != new_length, ECS_INTERNAL_ERROR, NULL);
            array->data = ecs_os_realloc(array->data, new_length * elem_size);
            ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);

            memset(ECS_OFFSET(array->data, length * elem_size), 0, (new_length - length) * elem_size);
            memmove(ECS_OFFSET(array->data, dif_offset * elem_size), array->data, 
                length * elem_size);
            memset(array->data, 0, dif_offset * elem_size);
            array->offset = new_offset;
        } else {
            ecs_assert(length != new_length, ECS_INTERNAL_ERROR, NULL);
            array->data = ecs_os_realloc(array->data, new_length * elem_size);
            ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);

            memset(ECS_OFFSET(array->data, length * elem_size), 0, 
                (new_length - length) * elem_size);
        }

        array->length = new_length;
    }

    ecs_assert((array->offset + array->length) <= PAGE_SIZE, 
        ECS_INTERNAL_ERROR, NULL);
    
    return ECS_OFFSET(array->data, (index - array->offset) * elem_size);
}

static
void* array_get(
    array_t *array,
    ecs_size_t elem_size,
    uint32_t index)
{
    uint16_t offset = array->offset;
    uint32_t length = array->length;

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
    uint16_t *addr,
    int count) 
{
    int32_t i;
    for (i = count; i > 0; i --) {
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
    uint16_t *addr,
    int count)
{
    int32_t i;
    for (i = count; i > 0; i --) {
        p = array_ensure(&p->pages, ECS_SIZEOF(page_t), addr[i]);
        ecs_assert(p != NULL, ECS_INTERNAL_ERROR, NULL);
    }

    return p;
}

static
int16_t page_count(
    uint64_t index)
{
    return 1 +
        (index > 0x00000000FFFF0000) +
        (index > 0x0000FFFF00000000);
}

static
uint32_t page_free(
    page_t *p)
{
    uint32_t result = sizeof(page_t);

    if (p->data.data) {
        result += p->data.length * sizeof(int);
        ecs_os_free(p->data.data);
    }

    if (p->pages.data) {
        uint32_t i;
        for (i = 0; i < p->pages.length; i ++) {
            result += page_free(array_get(&p->pages, ECS_SIZEOF(page_t), 
                i + p->pages.offset));
        }

        ecs_os_free(p->pages.data);
    }

    return result;
}

ecs_paged_t* _ecs_paged16_new(
    ecs_size_t elem_size)
{
    ecs_paged_t *result = ecs_os_calloc(sizeof(ecs_paged_t));
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY, NULL);
    result->first_65k = ecs_os_calloc(elem_size * 65536);
    return result;
}

uint32_t ecs_paged16_free(
    ecs_paged_t *paged)
{
    uint32_t result = page_free(&paged->root);
    result += sizeof(int) * 65536;
    result += sizeof(ecs_paged_t);

    ecs_os_free(paged->first_65k);
    ecs_os_free(paged);
    return result;
}

void _ecs_paged16_set(
    ecs_paged_t *paged,
    ecs_size_t elem_size,
    uint64_t index,
    void *value)
{
    void *data;
    if (index < 65536) {
        data = ECS_OFFSET(paged->first_65k, elem_size * index);
    } else {
        addr_t addr = to_addr(index);
        page_t *p = get_or_create_page(&paged->root, addr.value, page_count(index));
        ecs_assert(p != NULL, ECS_INTERNAL_ERROR, NULL);
        data = array_ensure(&p->data, elem_size, addr.value[0]);
        ecs_assert(data != NULL, ECS_INTERNAL_ERROR, NULL);
    }
    ecs_os_memcpy(data, value, elem_size);
}

void* _ecs_paged16_get(
    ecs_paged_t *paged,
    ecs_size_t elem_size,
    uint64_t index)
{
    if (index < 65536) {
        return ECS_OFFSET(paged->first_65k, elem_size * index);
    } else {
        addr_t addr = to_addr(index);
        page_t *p = get_page(&paged->root, addr.value, page_count(index));
        if (!p) {
            return NULL;
        }
        return array_get(&p->data, elem_size, addr.value[0]);
    }
}
