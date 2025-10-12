#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * @file hashmap.h
 * @brief 一个类型安全的、仅头文件的、C语言泛型哈希表实现 (C-OOP-Container)。
 *
 * 本库以面向对象的思想为核心，通过编译期宏生成代码，旨在提供现代C++ STL般的
 * 便利性，同时保持C语言的性能与控制力。
 *
 * @version 1.0
 * @date 2025-10-13
 */

// --- Internal Helper Functions ---
static int Hashmap_hash_cstr(const char* key) {
    const char* str = (const char*) key;
    int hash = 0;
    while (*str != '\0') {
        hash = hash * 31 + *str;
        str++;
    }
    return hash;
}
// --- Internal Helper Functions ---
static int Hashmap_hash_int64(long long key) { 
    union {
        long long llval;
        int parts[2];
    } u = {key};
    return u.parts[0] ^ u.parts[1];
}
// --- Internal Helper Functions ---
static int Hashmap_hash_uint64(unsigned long long key) { 
    union {             
        unsigned long long llval;
        int parts[2];
    } u = {key};
    return u.parts[0] ^ u.parts[1];
}
// --- Internal Helper Functions ---
static int Hashmap_hash_float64(double key) {
    union {             
        double lfval;
        int parts[2];
    } u = {key};
    return u.parts[0] ^ u.parts[1];
}
// --- Internal Helper Functions ---
static int Hashmap_hash_pointer(void* key) {
    union {             
        void* ptr;
        int parts[2];
    } u = {key};
    return u.parts[0] ^ u.parts[1];
}
// --- Internal Macros ---
#define __HASHMAP_LOAD_FACTOR 0.75f
// --- Internal Macros ---
#define __HASHMAP_DISPLAY_ELEMENT(stream, e)                                                                        \
_Generic(e,                                                                                                         \
    bool: fprintf(stream, "%s", e ? "true" : "false"),                                                              \
    char: fprintf(stream, "'%c'", e),                                                                               \
    short: fprintf(stream, "%d", e),                                                                                \
    int: fprintf(stream, "%d", e),                                                                                  \
    long: fprintf(stream, "%ld", e),                                                                                \
    long long: fprintf(stream, "%lld", e),                                                                          \
    unsigned char: fprintf(stream, "'%c'", e),                                                                      \
    unsigned short: fprintf(stream, "%u", e),                                                                       \
    unsigned int: fprintf(stream, "%u", e),                                                                         \
    unsigned long: fprintf(stream, "%lu", e),                                                                       \
    unsigned long long: fprintf(stream, "%llu", e),                                                                 \
    float: fprintf(stream, "%f", e),                                                                                \
    double: fprintf(stream, "%f", e),                                                                               \
    long double: fprintf(stream, "%Lf", e),                                                                         \
    const char*: fprintf(stream, "\"%s\"", e),                                                                      \
    default: fprintf(stream, "0x%p", e)                                                                             \
);                                                                                                                  \

// === 公共API: 定义宏 ===

/**
 * @brief 为指定的键值对类型定义一个具有默认行为的新哈希表。
 *
 * 这是创建哈希表的主要宏，适用于C语言的基本类型（整型、浮点型、指针、const char*），
 * 可以“开箱即用”。它会自动生成类型安全的哈希、比较和显示函数。
 *
 * @note **重要提示**: `K` 和 `V` 的类型名不能包含空格或星号 (`*`)。
 *       请使用 `typedef` 创建一个单一名词的别名。
 *       - **正确用法**: `typedef const char* cstr; HASHMAP_DEFINE(cstr, int)`
 *       - **错误用法**: `HASHMAP_DEFINE(const char*, int)`
 *
 * @param K 键的类型（必须是单个词）。
 * @param V 值的类型（必须是单个词）。
 *
 * @example
 * // 使用 typedef 为指针或多词类型创建别名
 * typedef const char* cstr;
 * typedef unsigned long ulong;
 * // 定义 Hashmap_cstr_int 及其相关函数
 * HASHMAP_DEFINE(cstr, int)
 * HASHMAP_DEFINE(ulong, cstr)
 */
#define HASHMAP_DEFINE(K, V)                                                                                        \
static int Hashmap_##K##_##V##_hash(K key) {                                                                        \
    union {                                                                                                         \
        K k;                                                                                                        \
        int ival;                                                                                                   \
        long long llval;                                                                                            \
        unsigned long long ullval;                                                                                  \
        const char* strval;                                                                                         \
        double lfval;                                                                                               \
        void* ptrval;                                                                                               \
    } u = {key};                                                                                                    \
    return _Generic(key,                                                                                            \
        bool: u.ival,                                                                                               \
        char: u.ival,                                                                                               \
        short: u.ival,                                                                                              \
        int: u.ival,                                                                                                \
        long: sizeof(long) == 4 ? u.ival : Hashmap_hash_int64(u.llval),                                             \
        long long: Hashmap_hash_int64(u.llval),                                                                     \
        unsigned char: u.ival,                                                                                      \
        unsigned short: u.ival,                                                                                     \
        unsigned int: u.ival,                                                                                       \
        unsigned long: sizeof(long) == 4 ? u.ival : Hashmap_hash_uint64(u.ullval),                                  \
        unsigned long long: Hashmap_hash_uint64(u.ullval),                                                          \
        float: u.ival,                                                                                              \
        double: Hashmap_hash_float64(u.lfval),                                                                      \
        long double: Hashmap_hash_float64(u.lfval),                                                                 \
        const char*: Hashmap_hash_cstr(u.strval),                                                                   \
        default: Hashmap_hash_pointer(u.ptrval)                                                                     \
    );                                                                                                              \
}                                                                                                                   \
static bool Hashmap_##K##_##V##_equals(K key1, K key2) {                                                            \
    union { K k; const char* strval; } u1 = {key1};                                                                 \
    union { K k; const char* strval; } u2 = {key2};                                                                 \
    return _Generic(key1,                                                                                           \
        const char*: strcmp(u1.strval, u2.strval) == 0,                                                             \
        default: u1.k == u2.k                                                                                       \
    );                                                                                                              \
}                                                                                                                   \
static void Hashmap_##K##_##V##_key_display(FILE* stream, K key) {                                                  \
    __HASHMAP_DISPLAY_ELEMENT(stream, key);                                                                         \
}                                                                                                                   \
static void Hashmap_##K##_##V##_value_display(FILE* stream, V value) {                                              \
    __HASHMAP_DISPLAY_ELEMENT(stream, value);                                                                       \
}                                                                                                                   \
HASHMAP_DEFINE_CUSTOM(K, V,                                                                                         \
    Hashmap_##K##_##V##_hash,                                                                                       \
    Hashmap_##K##_##V##_equals,                                                                                     \
    Hashmap_##K##_##V##_key_display,                                                                                \
    Hashmap_##K##_##V##_value_display                                                                               \
)                                                                                                                   \
/**
 * @brief 定义一个具有自定义行为函数的新哈希表。
 *
 * 这是创建任何哈希表类型的核心宏。它允许您为哈希、相等性检查和显示键/值
 * 提供自己的函数，使其适用于自定义结构体或复杂类型。
 *
 * @note **重要提示**: `K` 和 `V` 的类型名不能包含空格或星号 (`*`)。
 *       请使用 `typedef` 创建一个单一名词的别名。
 *
 * @param K 键的类型（必须是单个词）。
 * @param V 值的类型（必须是单个词）。
 * @param HashFn 用于哈希键的函数指针，类型为 `int (*)(K key)`。
 * @param EqualsFn 用于比较键的函数指针，类型为 `bool (*)(K key1, K key2)`。
 * @param DisplayKeyFn 用于打印键的函数指针，类型为 `void (*)(FILE* stream, K key)`。
 * @param DisplayValueFn 用于打印值的函数指针，类型为 `void (*)(FILE* stream, V value)`。
 */
#define HASHMAP_DEFINE_CUSTOM(K, V, HashFn, EqualsFn, DisplayKeyFn, DisplayValueFn)                                 \
                                                                                                                    \
typedef struct __Hashmap_##K##_##V Hashmap_##K##_##V;                                                               \
                                                                                                                    \
struct HashmapEntry_##K##_##V {                                                                                     \
    K key;                                                                                                          \
    V value;                                                                                                        \
    int hash;                                                                                                       \
    struct HashmapEntry_##K##_##V* next;                                                                            \
};                                                                                                                  \
                                                                                                                    \
struct HashmapIterator_##K##_##V {                                                                                  \
    Hashmap_##K##_##V* map;                                                                                         \
    int index;                                                                                                      \
    struct HashmapEntry_##K##_##V* entry;                                                                           \
};                                                                                                                  \
                                                                                                                    \
struct Hashmap_##K##_##V##_Functions {                                                                              \
    int (*hash)(K key);                                                                                             \
    bool (*equals)(K key1, K key2);                                                                                 \
    void (*display_key)(FILE* stream, K key);                                                                       \
    void (*display_value)(FILE* stream, V value);                                                                   \
    void (*display)(Hashmap_##K##_##V* self, FILE* stream);                                                         \
    void (*put)(Hashmap_##K##_##V* self, K key, V value);                                                           \
    const V* (*get)(Hashmap_##K##_##V* self, K key);                                                                \
    bool (*remove)(Hashmap_##K##_##V* self, K key);                                                                 \
    bool (*contains)(Hashmap_##K##_##V* self, K key);                                                               \
    void (*clear)(Hashmap_##K##_##V* self);                                                                         \
    struct HashmapIterator_##K##_##V (*get_iterator)(Hashmap_##K##_##V* self);                                      \
    bool (*iterator_next)(struct HashmapIterator_##K##_##V* self);                                                  \
    const K* (*iterator_current_key)(struct HashmapIterator_##K##_##V* self);                                       \
    const V* (*iterator_current_value)(struct HashmapIterator_##K##_##V* self);                                     \
    void (*free)(Hashmap_##K##_##V* self);                                                                          \
};                                                                                                                  \
                                                                                                                    \
struct __Hashmap_##K##_##V {                                                                                        \
    const struct Hashmap_##K##_##V##_Functions* fns;                                                                \
    struct HashmapEntry_##K##_##V** entries;                                                                        \
    int size;                                                                                                       \
    int capacity;                                                                                                   \
};                                                                                                                  \
                                                                                                                    \
static void Hashmap_##K##_##V##_display(Hashmap_##K##_##V* self, FILE* stream) {                                    \
    fprintf(stream, "{");                                                                                           \
    int count = 0;                                                                                                  \
    for (int i = 0; i < self->capacity; i++) {                                                                      \
        struct HashmapEntry_##K##_##V* entry = self->entries[i];                                                    \
        while (entry != NULL) {                                                                                     \
            self->fns->display_key(stream, entry->key);                                                             \
            fprintf(stream, ": ");                                                                                  \
            self->fns->display_value(stream, entry->value);                                                         \
            if (count < self->size - 1) {                                                                           \
                fprintf(stream, ", ");                                                                              \
            }                                                                                                       \
            count++;                                                                                                \
            entry = entry->next;                                                                                    \
        }                                                                                                           \
    }                                                                                                               \
    fprintf(stream, "}");                                                                                           \
}                                                                                                                   \
                                                                                                                    \
static void Hashmap_##K##_##V##_resize(Hashmap_##K##_##V* self) {                                                   \
    self->capacity *= 2;                                                                                            \
    struct HashmapEntry_##K##_##V** old_entries = self->entries;                                                    \
    self->entries =                                                                                                 \
        (struct HashmapEntry_##K##_##V**) malloc(self->capacity * sizeof(struct HashmapEntry_##K##_##V*));          \
    for (int i = 0; i < self->capacity; i++) {                                                                      \
        self->entries[i] = NULL;                                                                                    \
    }                                                                                                               \
    for (int i = 0; i < self->capacity / 2; i++) {                                                                  \
        struct HashmapEntry_##K##_##V* entry = old_entries[i];                                                      \
        while (entry != NULL) {                                                                                     \
            int index = entry->hash & (self->capacity - 1);                                                         \
            struct HashmapEntry_##K##_##V* next_entry = entry->next;                                                \
            entry->next = self->entries[index];                                                                     \
            self->entries[index] = entry;                                                                           \
            entry = next_entry;                                                                                     \
        }                                                                                                           \
    }                                                                                                               \
    free(old_entries);                                                                                              \
}                                                                                                                   \
                                                                                                                    \
static void Hashmap_##K##_##V##_put(Hashmap_##K##_##V* self, K key, V value) {                                      \
    if (self->size >= self->capacity * __HASHMAP_LOAD_FACTOR) {                                                     \
        Hashmap_##K##_##V##_resize(self);                                                                           \
    }                                                                                                               \
    int hash = self->fns->hash(key);                                                                                \
    int index = hash & (self->capacity - 1);                                                                        \
    struct HashmapEntry_##K##_##V* entry = self->entries[index];                                                    \
    struct HashmapEntry_##K##_##V* last_entry = self->entries[index];                                               \
    while (entry != NULL) {                                                                                         \
        if (entry->hash == hash && self->fns->equals(entry->key, key)) {                                            \
            entry->value = value;                                                                                   \
            return;                                                                                                 \
        }                                                                                                           \
        last_entry = entry;                                                                                         \
        entry = entry->next;                                                                                        \
    }                                                                                                               \
    entry = (struct HashmapEntry_##K##_##V*) malloc(sizeof(struct HashmapEntry_##K##_##V));                         \
    entry->key = key;                                                                                               \
    entry->value = value;                                                                                           \
    entry->next = NULL;                                                                                             \
    entry->hash = hash;                                                                                             \
    if (last_entry == NULL) {                                                                                       \
        self->entries[index] = entry;                                                                               \
    } else {                                                                                                        \
        last_entry->next = entry;                                                                                   \
    }                                                                                                               \
    self->size++;                                                                                                   \
}                                                                                                                   \
                                                                                                                    \
static const V* Hashmap_##K##_##V##_get(Hashmap_##K##_##V* self, K key) {                                           \
    int hash = self->fns->hash(key);                                                                                \
    int index = hash & (self->capacity - 1);                                                                        \
    struct HashmapEntry_##K##_##V* entry = self->entries[index];                                                    \
    while (entry != NULL) {                                                                                         \
        if (self->fns->equals(entry->key, key)) {                                                                   \
            return &entry->value;                                                                                   \
        }                                                                                                           \
        entry = entry->next;                                                                                        \
    }                                                                                                               \
    return NULL;                                                                                                    \
}                                                                                                                   \
                                                                                                                    \
static bool Hashmap_##K##_##V##_remove(Hashmap_##K##_##V* self, K key) {                                            \
    int hash = self->fns->hash(key);                                                                                \
    int index = hash & (self->capacity - 1);                                                                        \
    struct HashmapEntry_##K##_##V* entry = self->entries[index];                                                    \
    struct HashmapEntry_##K##_##V* prev = NULL;                                                                     \
    while (entry != NULL) {                                                                                         \
        if (entry->hash == hash && self->fns->equals(entry->key, key)) {                                            \
            if (prev == NULL) {                                                                                     \
                self->entries[index] = entry->next;                                                                 \
            } else {                                                                                                \
                prev->next = entry->next;                                                                           \
            }                                                                                                       \
            free(entry);                                                                                            \
            self->size--;                                                                                           \
            return true;                                                                                            \
        }                                                                                                           \
        prev = entry;                                                                                               \
        entry = entry->next;                                                                                        \
    }                                                                                                               \
    return false;                                                                                                   \
}                                                                                                                   \
                                                                                                                    \
static bool Hashmap_##K##_##V##_contains(Hashmap_##K##_##V* self, K key) {                                          \
    return Hashmap_##K##_##V##_get(self, key) != NULL;                                                              \
}                                                                                                                   \
                                                                                                                    \
static void Hashmap_##K##_##V##_clear(Hashmap_##K##_##V* self) {                                                    \
    for (int i = 0; i < self->capacity; i++) {                                                                      \
        struct HashmapEntry_##K##_##V* entry = self->entries[i];                                                    \
        while (entry != NULL) {                                                                                     \
            struct HashmapEntry_##K##_##V* next = entry->next;                                                      \
            free(entry);                                                                                            \
            entry = next;                                                                                           \
        }                                                                                                           \
        self->entries[i] = NULL;                                                                                    \
    }                                                                                                               \
    self->size = 0;                                                                                                 \
}                                                                                                                   \
                                                                                                                    \
static struct HashmapIterator_##K##_##V Hashmap_##K##_##V##_get_iterator(Hashmap_##K##_##V* self) {                 \
    struct HashmapIterator_##K##_##V iter = {                                                                       \
        .map = self,                                                                                                \
        .index = 0,                                                                                                 \
        .entry = NULL                                                                                               \
    };                                                                                                              \
    return iter;                                                                                                    \
}                                                                                                                   \
                                                                                                                    \
static bool Hashmap_##K##_##V##_iterator_next(struct HashmapIterator_##K##_##V* self) {                             \
    if (self->entry != NULL && self->entry->next != NULL) {                                                         \
        self->entry = self->entry->next;                                                                            \
        return true;                                                                                                \
    }                                                                                                               \
    while (self->index < self->map->capacity) {                                                                     \
        self->entry = self->map->entries[self->index];                                                              \
        self->index++;                                                                                              \
        if (self->entry != NULL) {                                                                                  \
            return true;                                                                                            \
        }                                                                                                           \
    }                                                                                                               \
    return false;                                                                                                   \
}                                                                                                                   \
                                                                                                                    \
static const K* Hashmap_##K##_##V##_iterator_current_key(struct HashmapIterator_##K##_##V* self) {                  \
    return &self->entry->key;                                                                                       \
}                                                                                                                   \
                                                                                                                    \
static const V* Hashmap_##K##_##V##_iterator_current_value(struct HashmapIterator_##K##_##V* self) {                \
    return &self->entry->value;                                                                                     \
}                                                                                                                   \
                                                                                                                    \
static void Hashmap_##K##_##V##_free(Hashmap_##K##_##V* self) {                                                     \
    for (int i = 0; i < self->capacity; i++) {                                                                      \
        struct HashmapEntry_##K##_##V* entry = self->entries[i];                                                    \
        while (entry != NULL) {                                                                                     \
            struct HashmapEntry_##K##_##V* next = entry->next;                                                      \
            free(entry);                                                                                            \
            entry = next;                                                                                           \
        }                                                                                                           \
    }                                                                                                               \
    free(self->entries);                                                                                            \
    free(self);                                                                                                     \
}                                                                                                                   \
                                                                                                                    \
const struct Hashmap_##K##_##V##_Functions HASHMAP_##K##V##FUNCTIONS = {                                            \
    .hash = HashFn,                                                                                                 \
    .equals = EqualsFn,                                                                                             \
    .display_key = DisplayKeyFn,                                                                                    \
    .display_value = DisplayValueFn,                                                                                \
    .display = Hashmap_##K##_##V##_display,                                                                         \
    .put = Hashmap_##K##_##V##_put,                                                                                 \
    .remove = Hashmap_##K##_##V##_remove,                                                                           \
    .contains = Hashmap_##K##_##V##_contains,                                                                       \
    .clear = Hashmap_##K##_##V##_clear,                                                                             \
    .get = Hashmap_##K##_##V##_get,                                                                                 \
    .get_iterator = Hashmap_##K##_##V##_get_iterator,                                                               \
    .iterator_next = Hashmap_##K##_##V##_iterator_next,                                                             \
    .iterator_current_key = Hashmap_##K##_##V##_iterator_current_key,                                               \
    .iterator_current_value = Hashmap_##K##_##V##_iterator_current_value,                                           \
    .free = Hashmap_##K##_##V##_free,                                                                               \
};                                                                                                                  \
                                                                                                                    \
static Hashmap_##K##_##V* Hashmap_##K##_##V##_new(int capacity) {                                                   \
    Hashmap_##K##_##V* self = (Hashmap_##K##_##V*) malloc(sizeof(Hashmap_##K##_##V));                               \
    self->fns = &HASHMAP_##K##V##FUNCTIONS;                                                                         \
    self->entries = (struct HashmapEntry_##K##_##V**) malloc(capacity * sizeof(struct HashmapEntry_##K##_##V*));    \
    for (int i = 0; i < capacity; i++) {                                                                            \
        self->entries[i] = NULL;                                                                                    \
    }                                                                                                               \
    self->size = 0;                                                                                                 \
    self->capacity = capacity;                                                                                      \
    return self;                                                                                                    \
}                                                                                                                   \

// === 公共API: 类型与构造函数宏 ===

/**
 * @brief 声明一个指向特定哈希表类型的指针。
 * @param K 在 HASHMAP_DEFINE 中使用的键类型。
 * @param V 在 HASHMAP_DEFINE 中使用的值类型。
 * @example hashmap(cstr, int) my_map;
 */
#define hashmap(K, V) Hashmap_##K##_##V*

/**
 * @brief 创建一个具有默认初始容量 (16) 的新哈希表。
 * @param K 键的类型。
 * @param V 值的类型。
 * @return 指向新创建的哈希表的指针。
 * @example my_map = hashmap_new(cstr, int);
 */
#define hashmap_new(K, V) Hashmap_##K##_##V##_new(16)

/**
 * @brief 创建一个具有指定初始容量的新哈希表。
 *
 * 容量**必须**是2的幂。此项将在运行时进行检查。
 *
 * @param K 键的类型。
 * @param V 值的类型。
 * @param capacity 初始容量，必须是2的幂。
 * @return 指向新创建的哈希表的指针。
 * @example my_map = hashmap_new_with_capacity(cstr, int, 1024);
 */
#define hashmap_new_with_capacity(K, V, capacity) ({                                                                \
    typeof(capacity) _capacity = (capacity);                                                                        \
    !(_capacity > 0 && (_capacity & (_capacity - 1)) == 0) ? (                                                      \
        fprintf(stderr, "%s:%d: HashMap capacity must be a power of two.", __FILE__, __LINE__),                     \
        fflush(stderr),                                                                                             \
        _Exit(-1),                                                                                                  \
        NULL                                                                                                        \
    ) : Hashmap_##K##_##V##_new(_capacity);                                                                         \
})

// === 公共API: 类型与构造函数宏 ===
/**
 * @brief 声明一个指向特定哈希表类型的指针。
 * @param K 在 HASHMAP_DEFINE 中使用的键类型。
 * @param V 在 HASHMAP_DEFINE 中使用的值类型。
 * @example hashmap(cstr, int) my_map;
 */
#define hashmap(K, V) Hashmap_##K##_##V*
/**
 * @brief 创建一个具有默认初始容量 (16) 的新哈希表。
 * @param K 键的类型。
 * @param V 值的类型。
 * @return 指向新创建的哈希表的指针。
 * @example my_map = hashmap_new(cstr, int);
 */
#define hashmap_new(K, V) Hashmap_##K##_##V##_new(16)
/**
 * @brief 创建一个具有指定初始容量的新哈希表。
 *
 * 容量**必须**是2的幂。此项将在运行时进行检查，若不满足则程序会中止。
 *
 * @param K 键的类型。
 * @param V 值的类型。
 * @param capacity 初始容量，必须是2的幂。
 * @return 指向新创建的哈希表的指针。
 * @example my_map = hashmap_new_with_capacity(cstr, int, 1024);
 */
#define hashmap_new_with_capacity(K, V, capacity) ({                                                                \
    typeof(capacity) _capacity = (capacity);                                                                        \
    !(_capacity > 0 && (_capacity & (_capacity - 1)) == 0) ? (                                                      \
        fprintf(stderr, "%s:%d: HashMap capacity must be a power of two.", __FILE__, __LINE__),                     \
        fflush(stderr),                                                                                             \
        _Exit(-1),                                                                                                  \
        NULL                                                                                                        \
    ) : Hashmap_##K##_##V##_new(_capacity);                                                                         \
})
// === 公共API: 核心操作宏 ===
/**
 * @brief 在哈希表中插入或更新一个键值对。
 *
 * 如果键已存在，则更新其值。否则，创建一个新条目。
 * 本宏使用可变参数 `...` 来接收 `value`，以支持复合字面量等包含逗号的值类型。
 *
 * @param map (hashmap(K,V)) 哈希表实例。
 * @param key (K) 键。
 * @param ... (V value) 与键关联的值。
 *
 * @example
 * hashmap_put(my_map, "学号-01", 101);
 * hashmap_put(roster_map, "Alice", (Student){101, 95.5f});
 */
#define hashmap_put(map, key, ...) (map)->fns->put((map), (key), __VA_ARGS__)
/**
 * @brief 检索与给定键关联的值。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @param key (K) 要查找的键。
 * @return (const V*) 如果找到键，则返回一个指向值的只读指针；否则返回 NULL。
 * @example const int* val = hashmap_get(my_map, "hello"); if (val) { printf("%d", *val); }
 */
#define hashmap_get(map, key) (map)->fns->get((map), (key))
/**
 * @brief 从哈希表中移除一个键值对。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @param key (K) 要移除条目的键。
 * @return (bool) 如果成功移除了一个元素，则返回 `true`；否则返回 `false`。
 * @example bool removed = hashmap_remove(my_map, "hello");
 */
#define hashmap_remove(map, key) (map)->fns->remove((map), (key))
/**
 * @brief 检查哈希表是否包含指定的键。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @param key (K) 要检查的键。
 * @return (bool) 如果键存在，则返回 `true`；否则返回 `false`。
 * @example if (hashmap_contains(my_map, "world")) { ... }
 */
#define hashmap_contains(map, key) (map)->fns->contains((map), (key))
// === 公共API: 工具与生命周期宏 ===
/**
 * @brief 将哈希表的内容显示到给定的文件流。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @param stream (FILE*) 输出流 (例如, stdout, stderr, 或一个文件指针)。
 * @example hashmap_display(my_map, stdout); // 输出: {"key1": val1, "key2": val2}
 */
#define hashmap_display(map, stream) (map)->fns->display((map), (stream))
/**
 * @brief 返回哈希表中键值对的数量。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @return (int) 哈希表的当前大小。
 * @example int count = hashmap_size(my_map);
 */
#define hashmap_size(map) (map)->size
/**
 * @brief 从哈希表中移除所有键值对，使其变为空。
 * 此操作不会释放哈希表结构体本身。对于持有动态资源的值，需要用户自行处理内存释放。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @example hashmap_clear(my_map);
 */
#define hashmap_clear(map) (map)->fns->clear(map)
/**
 * @brief 释放与哈希表相关的所有内存。
 * 包括所有条目、内部条目数组以及哈希表结构体本身。
 * 调用后，该哈希表指针将变为无效。对于持有动态资源的值，需要用户在调用此前手动释放。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @example hashmap_free(my_map);
 */
#define hashmap_free(map) (map)->fns->free(map)
// === 公共API: 迭代器宏 ===
/**
 * @brief 声明一个哈希表迭代器变量。
 * @param K 键的类型。
 * @param V 值的类型。
 * @example hashmap_iterator(cstr, int) it;
 */
#define hashmap_iterator(K, V) struct HashmapIterator_##K##_##V
/**
 * @brief 为哈希表创建一个迭代器。
 * 迭代器初始位置在第一个元素之前。
 * @param map (hashmap(K,V)) 哈希表实例。
 * @return (hashmap_iterator(K,V)) 一个用于该哈希表的迭代器。
 * @example hashmap_iterator(cstr, int) it = hashmap_get_iterator(my_map);
 */
#define hashmap_get_iterator(map) (map)->fns->get_iterator(map)
/**
 * @brief 将迭代器推进到哈希表中的下一个元素。
 * @param iter (hashmap_iterator(K,V)*) 指向迭代器的指针。
 * @return (bool) 如果迭代器成功指向一个有效元素，则返回 `true`；如果已到达末尾，则返回 `false`。
 * @example while (hashmap_iterator_next(&it)) { ... }
 */
#define hashmap_iterator_next(iter) (iter).map->fns->iterator_next(&(iter))
/**
 * @brief 检索迭代器当前位置的键。
 * 只有在成功调用 `hashmap_iterator_next` 后才能调用此宏。
 * @param iter (hashmap_iterator(K,V)*) 指向迭代器的指针。
 * @return (const K*) 指向当前键的只读指针。
 * @example const cstr* key = hashmap_iterator_current_key(&it);
 */
#define hashmap_iterator_current_key(iter) (iter).map->fns->iterator_current_key(&(iter))
/**
 * @brief 检索迭代器当前位置的值。
 * 只有在成功调用 `hashmap_iterator_next` 后才能调用此宏。
 * @param iter (hashmap_iterator(K,V)*) 指向迭代器的指针。
 * @return (const V*) 指向当前值的只读指针。
 * @example const int* val = hashmap_iterator_current_value(&it);
 */
#define hashmap_iterator_current_value(iter) (iter).map->fns->iterator_current_value(&(iter))

#endif // HASHMAP_H