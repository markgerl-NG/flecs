#include "flecs.h"

#define PAGE_SIZE (256)

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
    void *first_65k;
};

// static
// addr_t to_addr(
//     uint64_t id) 
// {
//     union {
//         addr_t addr;
//         uint64_t id;
//     } u = {.id = id};
//     return u.addr;
// }

static
addr_t to_addr(
    uint64_t id) 
{
    union {
        addr_t addr;
        uint64_t id;
    } u = {.id = id};

    union {
        addr_t addr;
        uint64_t id;
    } r;

    r.addr.value[0] = u.addr.value[0];
    r.addr.value[1] = u.addr.value[1];
    r.addr.value[2] = u.addr.value[2];
    r.addr.value[3] = u.addr.value[4];
    r.addr.value[4] = u.addr.value[5];
    r.addr.value[5] = u.addr.value[7];
    r.addr.value[6] = u.addr.value[3];
    r.addr.value[7] = u.addr.value[6];

    return r.addr;
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
        ecs_assert((array->offset + array->length) <= PAGE_SIZE, 
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
        ecs_assert((array->offset + array->length) <= PAGE_SIZE, 
            ECS_INTERNAL_ERROR, NULL);
    }

    return ECS_OFFSET(array->data, (index - offset) * elem_size);
}

// static
// void* array_ensure(
//     array_t *array,
//     ecs_size_t elem_size,
//     uint16_t index)
// {
//     uint16_t offset = array->offset;
//     uint32_t length = array->length;

//     if (!length) {
//         // uint16_t new_offset = next_pow_of_2(index) >> 1;
//         // uint32_t new_length = next_pow_of_2(index - new_offset + 1);
//         // array->offset = new_offset;
//         // array->length = new_length;
        
//         // ecs_assert(length != new_length, ECS_INTERNAL_ERROR, NULL);
//         // array->data = ecs_os_calloc(new_length * elem_size);
//         // ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);
//         // printf("new (offset = %u, length = %u) [%u]\n", array->offset, array->length, index);

//         array->length = 1;
//         array->offset = index;
//         array->data = ecs_os_calloc(elem_size);
//         ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);
//         return array->data; 

//     } else if (index < offset) {
//         uint32_t new_offset = next_pow_of_2(index) >> 1;
//         uint32_t dif_offset = offset - new_offset;
//         uint32_t new_length = next_pow_of_2(length + dif_offset);
        
//         if ((new_offset + new_length) > PAGE_SIZE) {
//             new_offset = 0;
//             new_length = PAGE_SIZE;
//             dif_offset = offset;
//         }

//         ecs_assert(length != new_length, ECS_INTERNAL_ERROR, NULL);
//         array->data = ecs_os_realloc(array->data, new_length * elem_size);
//         ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);

//         memset(ECS_OFFSET(array->data, length * elem_size), 0, (new_length - length) * elem_size);
//         memmove(ECS_OFFSET(array->data, dif_offset * elem_size), array->data, 
//             length * elem_size);
//         memset(array->data, 0, dif_offset * elem_size);            
//         array->offset = new_offset;
//         array->length = new_length;

//         // printf("lxp (offset = %u, length = %u) <= (%u, %u) [%u]\n", 
//         //     array->offset, array->length,
//         //     offset, length, index);

//     } else if (index >= (offset + length)) {
//         uint32_t new_length = next_pow_of_2(index + offset + 1);
//         if ((new_length + offset) > PAGE_SIZE) {
//             uint32_t new_offset = next_pow_of_2(offset - ((new_length + offset) - PAGE_SIZE)) >> 1;
//             uint32_t dif_offset = offset - new_offset;
//             new_length = next_pow_of_2(new_offset + index + 1);
            
//             ecs_assert(length != new_length, ECS_INTERNAL_ERROR, NULL);
//             array->data = ecs_os_realloc(array->data, new_length * elem_size);
//             ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);

//             memset(ECS_OFFSET(array->data, length * elem_size), 0, (new_length - length) * elem_size);
//             memmove(ECS_OFFSET(array->data, dif_offset * elem_size), array->data, 
//                 length * elem_size);
//             memset(array->data, 0, dif_offset * elem_size);
//             array->offset = new_offset;
//         } else {
//             ecs_assert(length != new_length, ECS_INTERNAL_ERROR, NULL);
//             array->data = ecs_os_realloc(array->data, new_length * elem_size);
//             ecs_assert(array->data != NULL, ECS_OUT_OF_MEMORY, NULL);

//             memset(ECS_OFFSET(array->data, length * elem_size), 0, 
//                 (new_length - length) * elem_size);
//         }

//         array->length = new_length;

//         // printf("rxp (offset = %u, length = %u) <= (%u, %u) [%u]\n", 
//         //     array->offset, array->length,
//         //     offset, length, index);
//     }

//     ecs_assert((array->offset + array->length) <= PAGE_SIZE, 
//         ECS_INTERNAL_ERROR, NULL);
    
//     return ECS_OFFSET(array->data, (index - array->offset) * elem_size);
// }

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
    uint8_t *addr,
    int count)
{
    int32_t i;
    for (i = count; i > 0; i --) {
        uint8_t index = addr[i];
        p = array_ensure(&p->pages, ECS_SIZEOF(page_t), index);
        ecs_assert(p != NULL, ECS_INTERNAL_ERROR, NULL);
    }

    return p;
}

    // return
    //     (index > 0x00000000000000FF) +
    //     (index > 0x000000000000FF00) +
    //     (index > 0x0000000000FF0000) +
    //     (index > 0x00000000FF000000) +
    //     (index > 0x000000FF00000000) +
    //     (index > 0x0000FF0000000000) +
    //     (index > 0x00FF000000000000) ;

    // return 1 +
    //     (index > 0x000000000000FF00) +
    //     (index > 0x0000000000FF0000) +
    //     (index > 0x00000000FF000000) +
    //     (index > 0x00FFFFFF00000000) * 3;

static
int8_t page_count(
    uint64_t index)
{
    return 1 +
        (index > 0x000000000000FF00) +
        (index > 0x0000000000FF0000) +
        (index > 0x00000000FF000000) +
        (index > 0x00FFFFFF00000000) * 3;
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
        uint16_t i;
        for (i = 0; i < p->pages.length; i ++) {
            result += page_free(array_get(&p->pages, ECS_SIZEOF(page_t), 
                i + p->pages.offset));
        }

        ecs_os_free(p->pages.data);
    }

    return result;
}

ecs_paged_t* _ecs_paged_new(
    ecs_size_t elem_size)
{
    ecs_paged_t *result = ecs_os_calloc(sizeof(ecs_paged_t));
    ecs_assert(result != NULL, ECS_OUT_OF_MEMORY, NULL);
    result->first_65k = ecs_os_calloc(elem_size * 65536);
    return result;
}

uint32_t ecs_paged_free(
    ecs_paged_t *paged)
{
    uint32_t result = page_free(&paged->root);
    result += sizeof(int) * 65536;
    result += sizeof(ecs_paged_t);

    ecs_os_free(paged->first_65k);
    ecs_os_free(paged);
    return result;
}

void _ecs_paged_set(
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

void* _ecs_paged_get(
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
