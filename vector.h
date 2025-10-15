#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdbool.h>

/**
 * @file vector.h
 * @brief 一个类型安全的、仅头文件的、C语言泛型动态数组实现 (C-OOP-Container)。
 *
 * 本库以面向对象的思想为核心，通过编译期宏生成代码，旨在提供一个简单、高效且
 * 易于使用的动态数组容器。
 *
 * @version 1.0
 * @date 2025-10-13
 */

// === 公共API: 定义宏 ===

/**
 * @brief 为指定的元素类型定义一个具有默认行为的新向量。
 *
 * 这是创建向量的主要宏，适用于C语言的基本类型，可以“开箱即用”。
 * 它会自动生成类型安全的比较和显示函数。
 *
 * @note **重要提示**: `T` 的类型名不能包含空格或星号 (`*`)。
 *       请使用 `typedef` 创建一个单一名词的别名。
 *       - **正确用法**: `typedef unsigned int uint; VECTOR_DEFINE(uint)`
 *       - **错误用法**: `VECTOR_DEFINE(unsigned int)`
 *
 * @param T 元素类型（必须是单个词）。如果对结构体使用此宏，并在之后调用
 *          需要比较的函数（如 `vector_index_of`），将会产生编译错误。
 *          对于结构体，请使用 VECTOR_DEFINE_CUSTOM。
 *
 * @example
 * // 使用 typedef 为多词类型创建别名
 * typedef unsigned long ulong;
 * VECTOR_DEFINE(int)
 * VECTOR_DEFINE(ulong)
 */
#define VECTOR_DEFINE(T)                                                            \
static bool Vector_##T##_equals(T e1, T e2) { return e1 == e2; }                    \
static void Vector_##T##_display_element(FILE* stream, T e) {                       \
    _Generic(e,                                                                     \
        bool: fprintf(stream, "%s", e ? "true" : "false"),                          \
        char: fprintf(stream, "'%c'", e),                                           \
        short: fprintf(stream, "%d", e),                                            \
        int: fprintf(stream, "%d", e),                                              \
        long: fprintf(stream, "%ld", e),                                            \
        long long: fprintf(stream, "%lld", e),                                      \
        unsigned char: fprintf(stream, "'%c'", e),                                  \
        unsigned short: fprintf(stream, "%u", e),                                   \
        unsigned int: fprintf(stream, "%u", e),                                     \
        unsigned long: fprintf(stream, "%lu", e),                                   \
        unsigned long long: fprintf(stream, "%llu", e),                             \
        float: fprintf(stream, "%f", e),                                            \
        double: fprintf(stream, "%f", e),                                           \
        long double: fprintf(stream, "%Lf", e),                                     \
        const char*: fprintf(stream, "\"%s\"", e),                                  \
        default: fprintf(stream, "0x%p", e)                                         \
    );                                                                              \
}                                                                                   \
VECTOR_DEFINE_CUSTOM(T, Vector_##T##_equals, Vector_##T##_display_element)          \
/**
 * @brief 定义一个具有自定义行为函数的新向量。
 *
 * 这是创建任何向量类型的核心宏。它允许您为元素提供自己的
 * 相等性检查和显示函数。
 *
 * @note **重要提示**: `T` 的类型名不能包含空格或星号 (`*`)。
 *       请使用 `typedef` 创建一个单一名词的别名。
 *
 * @param T 元素类型（必须是单个词）。
 * @param EqualsFn 用于比较元素的函数指针，类型为 `bool (*)(T e1, T e2)`。
 * @param DisplayFn 用于打印元素的函数指针，类型为 `void (*)(FILE* stream, T e)`。
 */
#define VECTOR_DEFINE_CUSTOM(T, EqualsFn, DisplayFn)                                \
                                                                                    \
typedef struct _Vector_##T Vector_##T;                                              \
                                                                                    \
struct VectorIterator_##T {                                                         \
    Vector_##T* vec;                                                                \
    int index;                                                                      \
};                                                                                  \
                                                                                    \
struct Vector_##T##_Functions {                                                     \
    bool (*equals)(T e1, T e2);                                                     \
    void (*display_element)(FILE* stream, T e);                                     \
    void (*display)(Vector_##T* self, FILE* stream);                                \
    void (*push)(Vector_##T* self, T value);                                        \
    bool (*pop)(Vector_##T* self);                                                  \
    const T* (*get)(Vector_##T* self, int index);                                   \
    const T* (*last)(Vector_##T* self);                                             \
    bool (*remove)(Vector_##T* self, int index);                                    \
    int (*index_of)(Vector_##T* self, T value);                                     \
    bool (*remove_element)(Vector_##T* self, T value);                              \
    bool (*set)(Vector_##T* self, int index, T value);                              \
    bool (*insert)(Vector_##T* self, int index, T value);                           \
    bool (*contains)(Vector_##T* self, T value);                                    \
    void (*clear)(Vector_##T* self);                                                \
    struct VectorIterator_##T (*get_iterator)(Vector_##T* self);                    \
    bool (*iterator_next)(struct VectorIterator_##T* self);                         \
    const T* (*iterator_current)(struct VectorIterator_##T* self);                  \
    void (*free)(Vector_##T* self);                                                 \
};                                                                                  \
                                                                                    \
struct _Vector_##T {                                                                \
    const struct Vector_##T##_Functions* fns;                                       \
    T* data;                                                                        \
    int size;                                                                       \
    int capacity;                                                                   \
};                                                                                  \
                                                                                    \
static void Vector_##T##_display(Vector_##T* self, FILE* stream) {                  \
    fprintf(stream, "[");                                                           \
    for (int i = 0; i < self->size; i++) {                                          \
        self->fns->display_element(stream, self->data[i]);                          \
        if (i != self->size - 1) {                                                  \
            fprintf(stream, ", ");                                                  \
        }                                                                           \
    }                                                                               \
    fprintf(stream, "]");                                                           \
}                                                                                   \
                                                                                    \
static void Vector_##T##_push(Vector_##T* self, T value) {                          \
    if (self->size == self->capacity) {                                             \
        self->capacity *= 2;                                                        \
        self->data = (T*) realloc(self->data, self->capacity * sizeof(T));          \
    }                                                                               \
    self->data[self->size] = value;                                                 \
    self->size++;                                                                   \
}                                                                                   \
                                                                                    \
static bool Vector_##T##_pop(Vector_##T* self) {                                    \
    if (self->size > 0) {                                                           \
        self->size--;                                                               \
        return true;                                                                \
    }                                                                               \
    return false;                                                                   \
}                                                                                   \
                                                                                    \
static const T* Vector_##T##_last(Vector_##T* self) {                               \
    if (self->size > 0) {                                                           \
        return &self->data[self->size - 1];                                         \
    }                                                                               \
    return NULL;                                                                    \
}                                                                                   \
                                                                                    \
static const T* Vector_##T##_get(Vector_##T* self, int index) {                     \
    if (index < self->size) {                                                       \
        return &self->data[index];                                                  \
    }                                                                               \
    return NULL;                                                                    \
}                                                                                   \
                                                                                    \
static bool Vector_##T##_remove(Vector_##T* self, int index) {                      \
    if (index < 0 || index >= self->size) {                                         \
        return false;                                                               \
    }                                                                               \
    for (int i = index; i < self->size - 1; i++) {                                  \
        self->data[i] = self->data[i + 1];                                          \
    }                                                                               \
    self->size--;                                                                   \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static int Vector_##T##_index_of(Vector_##T* self, T value) {                       \
    for (int i = 0; i < self->size; i++) {                                          \
        if (self->fns->equals(self->data[i], value)) {                              \
            return i;                                                               \
        }                                                                           \
    }                                                                               \
    return -1;                                                                      \
}                                                                                   \
                                                                                    \
static bool Vector_##T##_remove_element(Vector_##T* self, T value) {                \
    return Vector_##T##_remove(self, Vector_##T##_index_of(self, value));           \
}                                                                                   \
                                                                                    \
static bool Vector_##T##_set(Vector_##T* self, int index, T value) {                \
    if (index < 0 || index >= self->size) {                                         \
        return false;                                                               \
    }                                                                               \
    self->data[index] = value;                                                      \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static bool Vector_##T##_insert(Vector_##T* self, int index, T value) {             \
    if (index < 0 || index > self->size) {                                          \
        return false;                                                               \
    }                                                                               \
    if (self->size == self->capacity) {                                             \
        self->capacity *= 2;                                                        \
        self->data = (T*) realloc(self->data, self->capacity * sizeof(T));          \
    }                                                                               \
    for (int i = self->size; i > index; i--) {                                      \
        self->data[i] = self->data[i - 1];                                          \
    }                                                                               \
    self->data[index] = value;                                                      \
    self->size++;                                                                   \
    return true;                                                                    \
}                                                                                   \
                                                                                    \
static bool Vector_##T##_contains(Vector_##T* self, T value) {                      \
    return Vector_##T##_index_of(self, value) != -1;                                \
}                                                                                   \
                                                                                    \
static void Vector_##T##_clear(Vector_##T* self) {                                  \
    self->size = 0;                                                                 \
}                                                                                   \
                                                                                    \
static struct VectorIterator_##T Vector_##T##_get_iterator(Vector_##T* self) {      \
    struct VectorIterator_##T iter = {                                              \
        .vec = self,                                                                \
        .index = -1                                                                 \
    };                                                                              \
    return iter;                                                                    \
}                                                                                   \
                                                                                    \
static bool Vector_##T##_iterator_next(struct VectorIterator_##T* self) {           \
    if (self->index < self->vec->size - 1) {                                        \
        self->index++;                                                              \
        return true;                                                                \
    }                                                                               \
    return false;                                                                   \
}                                                                                   \
                                                                                    \
static const T* Vector_##T##_iterator_current(struct VectorIterator_##T* self) {    \
    if (self->index >= 0 && self->index < self->vec->size) {                        \
        return &self->vec->data[self->index];                                       \
    }                                                                               \
    return NULL;                                                                    \
}                                                                                   \
                                                                                    \
static void Vector_##T##_free(Vector_##T* self) {                                   \
    free(self->data);                                                               \
    free(self);                                                                     \
}                                                                                   \
                                                                                    \
const static struct Vector_##T##_Functions VECTOR_##T##_FUNCTIONS = {               \
    .equals = EqualsFn,                                                             \
    .display_element = DisplayFn,                                                   \
    .display = Vector_##T##_display,                                                \
    .push = Vector_##T##_push,                                                      \
    .pop = Vector_##T##_pop,                                                        \
    .get = Vector_##T##_get,                                                        \
    .last = Vector_##T##_last,                                                      \
    .remove = Vector_##T##_remove,                                                  \
    .index_of = Vector_##T##_index_of,                                              \
    .remove_element = Vector_##T##_remove_element,                                  \
    .insert = Vector_##T##_insert,                                                  \
    .set = Vector_##T##_set,                                                        \
    .contains = Vector_##T##_contains,                                              \
    .clear = Vector_##T##_clear,                                                    \
    .get_iterator = Vector_##T##_get_iterator,                                      \
    .iterator_next = Vector_##T##_iterator_next,                                    \
    .iterator_current = Vector_##T##_iterator_current,                              \
    .free = Vector_##T##_free,                                                      \
};                                                                                  \
                                                                                    \
static Vector_##T* Vector_##T##_new(int capacity) {                                 \
    Vector_##T* self = (Vector_##T*) malloc(sizeof(Vector_##T));                    \
    self->fns = &VECTOR_##T##_FUNCTIONS;                                            \
    self->data = (T*) malloc(capacity * sizeof(T));                                 \
    self->size = 0;                                                                 \
    self->capacity = capacity;                                                      \
    return self;                                                                    \
}                                                                                   \



// === 公共API: 类型与构造函数宏 ===

/**
 * @brief 声明一个指向特定向量类型的指针。
 * @param T 在 VECTOR_DEFINE 中使用的元素类型。
 * @example vector(int) my_vec;
 */
#define vector(T) Vector_##T*

/**
 * @brief 创建一个具有默认初始容量 (10) 的新向量。
 * @param T 元素类型。
 * @return 指向新创建的向量的指针。
 * @example my_vec = vector_new(int);
 */
#define vector_new(T) Vector_##T##_new(10)

/**
 * @brief 创建一个具有指定初始容量的新向量。
 * @param T 元素类型。
 * @param capacity 初始容量。
 * @return 指向新创建的向量的指针。
 * @example my_vec = vector_new_with_capacity(int, 100);
 */
#define vector_new_with_capacity(T, capacity) Vector_##T##_new(capacity)


// === 公共API: 核心操作宏 ===

/**
 * @brief 将一个值追加到向量的末尾。
 *
 * 如果需要，向量的容量会自动增加。
 * 本宏使用可变参数 `...` 来接收 `value`，这是为了完美支持
 * 复合字面量 (Compound Literals) 作为参数，例如 `(Student){1, "Alice"}`。
 *
 * @param vec (vector(T)) 向量实例。
 * @param ... (T value) 要压入的值。请将要压入的单个值作为第二个参数传入。
 *
 * @example
 * vector_push(my_vec, 42);
 * vector_push(student_vec, (Student){101, "Alice"});
 */
#define vector_push(vec, ...) (vec)->fns->push((vec), __VA_ARGS__)

/**
 * @brief从向量中移除最后一个元素。
 * @param vec (vector(T)) 向量实例。
 * @return (bool) 如果成功移除了一个元素，则返回 `true`；如果向量已空，则返回 `false`。
 * @example vector_pop(my_vec);
 */
#define vector_pop(vec) (vec)->fns->pop(vec)

/**
 * @brief 检索特定索引处的元素。
 * @param vec (vector(T)) 向量实例。
 * @param index (int) 元素的零基索引。
 * @return (const T*) 如果索引有效，则返回指向元素的只读指针；否则返回 NULL。
 * @example const int* val = vector_get(my_vec, 0);
 */
#define vector_get(vec, index) (vec)->fns->get((vec), (index))

/**
 * @brief 检索向量的最后一个元素。
 * @param vec (vector(T)) 向量实例。
 * @return (const T*) 指向最后一个元素的只读指针；如果向量为空，则返回 NULL。
 * @example const int* last_val = vector_last(my_vec);
 */
#define vector_last(vec) (vec)->fns->last(vec)

/**
 * @brief 用一个新值更新特定索引处的元素。
 *
 * 本宏使用可变参数 `...` 来接收 `value`，以支持复合字面量。
 *
 * @param vec (vector(T)) 向量实例。
 * @param index (int) 要设置元素的零基索引。
 * @param ... (T value) 新的值。
 * @return (bool) 如果索引有效且元素被设置，则返回 `true`；否则返回 `false`。
 * @example vector_set(my_vec, 0, 99);
 */
#define vector_set(vec, index, ...) (vec)->fns->set((vec), (index), __VA_ARGS__)
/**
 * @brief 在特定索引处插入一个值，并将后续元素后移。
 *
 * 本宏使用可变参数 `...` 来接收 `value`，以支持复合字面量。
 *
 * @param vec (vector(T)) 向量实例。
 * @param index (int) 要插入位置的零基索引。
 * @param ... (T value) 要插入的值。
 * @return (bool) 如果插入成功，则返回 `true`；如果索引越界，则返回 `false`。
 * @example vector_insert(my_vec, 1, 123);
 */
#define vector_insert(vec, index, ...) (vec)->fns->insert((vec), (index), __VA_ARGS__)

/**
 * @brief 移除特定索引处的元素。
 * @param vec (vector(T)) 向量实例。
 * @param index (int) 要移除元素的索引。
 * @return (bool) 如果成功移除了一个元素，则返回 `true`；如果索引越界，则返回 `false`。
 * @example vector_remove(my_vec, 1);
 */
#define vector_remove(vec, index) (vec)->fns->remove((vec), (index))


// === 公共API: 搜索与查询宏 ===

/**
 * @brief 在向量中查找给定值的第一个索引。
 * 需要为元素类型T定义一个有效的 `equals` 函数。
 * @param vec (vector(T)) 向量实例。
 * @param value (T) 要搜索的值。
 * @return (int) 值的第一个出现位置的索引；如果未找到，则返回 -1。
 * @example int pos = vector_index_of(my_vec, 42);
 */
#define vector_index_of(vec, value) (vec)->fns->index_of((vec), (value))

/**
 * @brief 从向量中移除第一次出现的给定值。
 * @param vec (vector(T)) 向量实例。
 * @param value (T) 要移除的值。
 * @return (bool) 如果找到并移除了一个元素，则返回 `true`；否则返回 `false`。
 * @example vector_remove_element(my_vec, 42);
 */
#define vector_remove_element(vec, value) (vec)->fns->remove_element((vec), (value))

/**
 * @brief 检查向量是否包含特定值。
 * @param vec (vector(T)) 向量实例。
 * @param value (T) 要检查的值。
 * @return (bool) 如果值存在，则返回 `true`；否则返回 `false`。
 * @example if (vector_contains(my_vec, 99)) { ... }
 */
#define vector_contains(vec, value) (vec)->fns->contains((vec), (value))


// === 公共API: 工具与生命周期宏 ===

/**
 * @brief 将向量的内容显示到给定的文件流。
 * @param vec (vector(T)) 向量实例。
 * @param stream (FILE*) 输出流 (例如, stdout)。
 * @example vector_display(my_vec, stdout); // 输出: [elem1, elem2, elem3]
 */
#define vector_display(vec, stream) (vec)->fns->display((vec), (stream))

/**
 * @brief 返回向量中元素的数量。
 * @param vec (vector(T)) 向量实例。
 * @return (int) 向量的当前大小。
 * @example int count = vector_size(my_vec);
 */
#define vector_size(vec) (vec)->size

/**
 * @brief 从向量中移除所有元素，使其变为空。
 * 此操作不会释放向量结构体本身。
 * @param vec (vector(T)) 向量实例。
 * @example vector_clear(my_vec);
 */
#define vector_clear(vec) (vec)->fns->clear(vec)

/**
 * @brief 释放与向量相关的所有内存。
 * 包括内部数据数组和向量结构体本身。
 * 调用后，该向量指针将变为无效。
 * @param vec (vector(T)) 向量实例。
 * @example vector_free(my_vec);
 */
#define vector_free(vec) (vec)->fns->free(vec)


// === 公共API: 迭代器宏 ===

/**
 * @brief 声明一个向量迭代器变量。
 * @param T 元素类型。
 * @example vector_iterator(int) it;
 */
#define vector_iterator(T) struct VectorIterator_##T

/**
 * @brief 为向量创建一个迭代器。
 * 迭代器初始位置在第一个元素之前。
 * @param vec (vector(T)) 向量实例。
 * @return (vector_iterator(T)) 一个用于该向量的迭代器。
 * @example vector_iterator(int) it = vector_get_iterator(my_vec);
 */
#define vector_get_iterator(vec) (vec)->fns->get_iterator(vec)

/**
 * @brief 将迭代器推进到向量中的下一个元素。
 * @param iter (vector_iterator(T)*) 指向迭代器的指针。
 * @return (bool) 如果迭代器成功指向一个有效元素，则返回 `true`；如果已到达末尾，则返回 `false`。
 * @example while (vector_iterator_next(&it)) { ... }
 */
#define vector_iterator_next(iter) (iter).vec->fns->iterator_next(&(iter))

/**
 * @brief 检索迭代器当前位置的元素。
 * 只有在成功调用 `vector_iterator_next` 后才能调用此宏。
 * @param iter (vector_iterator(T)*) 指向迭代器的指针。
 * @return (const T*) 指向当前元素的只读指针。
 * @example const int* val = vector_iterator_current(&it);
 */
#define vector_iterator_current(iter) (iter).vec->fns->iterator_current(&(iter))

#endif // VECTOR_H