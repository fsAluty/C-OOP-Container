# C-OOP-Container C语言泛型容器库：`vector` 与 `hashmap`

## 概述

这是一个为C语言设计的、类型安全的、仅需头文件的泛型容器库。它提供了高性能的动态数组 (`vector`) 和哈希表 (`hashmap`)，其设计灵感来源于现代C++的便利性，同时保留了C语言的性能和控制力。

本项目旨在解决C语言在数据结构使用上的两大痛点：
1.  **缺乏泛型**：传统的C语言数据结构要么为每种类型手写一套实现，要么使用`void*`牺牲类型安全和性能。
2.  **API繁琐**：许多C库需要用户编写大量模板式的“胶水代码”才能使用。

本库通过精巧的宏和现代C语言特性 (C11 `_Generic`)，实现了“开箱即用”的体验。对于大部分基本类型，你无需任何额外配置，即可像在高级语言中一样方便地使用`vector`和`hashmap`。

## 核心特性

*   **真正的泛型**：通过编译期代码生成，为每种类型创建专门的、类型安全的数据结构，无`void*`带来的运行时开销。
*   **开箱即用**：内置对所有C语言基本算术类型、指针和字符串 (`const char*`) 的支持。无需为它们编写哈希、比较或打印函数。
*   **API简洁优雅**：提供了一套全小写的宏接口（如`vector_push`, `hashmap_get`），风格统一，简单易用。
*   **高度可扩展**：为自定义结构体提供了专门的`_CUSTOM`宏，允许你轻松传入自己的比较、哈希和显示函数。
*   **健壮的错误处理**：
    *   在编译期，对结构体等复杂类型使用默认的比较函数会直接报错，清晰地引导用户使用自定义函数。
    *   在运行时，对`hashmap`的容量进行检查，强制要求其为2的幂，确保哈希算法的高效性。
*   **清晰的文档**：所有公开的API宏都配有符合Doxygen规范的详细注释，解释了其功能、参数和使用限制，弥补了IDE无法提供宏类型提示的不足。
*   **仅头文件**：整个库只由 `vector.h` 和 `hashmap.h` 两个文件组成，可以非常方便地集成到任何项目中。

## 快速上手

### Vector (动态数组)

```c
#include <stdio.h>
#include "vector.h"

// 为 "const char*" 创建一个单一名词的别名
typedef const char* cstr;

// 1. 定义一个存储整数的vector类型
VECTOR_DEFINE(int);
// 2. 定义一个存储字符串的vector类型
VECTOR_DEFINE(cstr);

int main() {
    // === 整数Vector示例 ===
    vector(int) int_vec = vector_new(int);
    vector_push(int_vec, 10);
    vector_push(int_vec, 20);
    vector_push(int_vec, 30);

    printf("整数Vector: ");
    vector_display(int_vec, stdout); // 输出: [10, 20, 30]
    printf("\n");

    const int* val = vector_get(int_vec, 1);
    if (val) {
        printf("索引为1的元素是: %d\n", *val); // 输出: 20
    }
    vector_free(int_vec);


    // === 字符串Vector示例 ===
    vector(cstr) str_vec = vector_new(cstr);
    vector_push(str_vec, "你好");
    vector_push(str_vec, "世界");

    printf("字符串Vector: ");
    vector_display(str_vec, stdout); // 输出: ["你好", "世界"]
    printf("\n");
    vector_free(str_vec);

    return 0;
}
```

### HashMap (哈希表)

```c
#include <stdio.h>
#include "hashmap.h"

// 为 "const char*" 创建一个单一名词的别名
typedef const char* cstr;

// 1. 定义一个键为字符串、值为整数的哈希表类型
HASHMAP_DEFINE(cstr, int);

int main() {
    hashmap(cstr, int) map = hashmap_new_with_capacity(cstr, int, 8);

    hashmap_put(map, "一", 1);
    hashmap_put(map, "二", 2);
    hashmap_put(map, "三", 3);

    printf("哈希表: ");
    hashmap_display(map, stdout); // 输出: {"一": 1, "三": 3, "二": 2} (顺序不定)
    printf("\n");

    const int* val = hashmap_get(map, "二");
    if (val) {
        printf("键 '二' 对应的值是: %d\n", *val); // 输出: 2
    }

    hashmap_free(map);
    return 0;
}
```

## 设计哲学

本库的设计遵循以下原则：
1.  **简单优先**：提供简洁明了的API，隐藏内部实现的复杂性。
2.  **安全默认**：默认行为应尽可能安全。例如，对未知类型默认打印其地址而不是尝试解释其内容，对有歧义的`char*`采取保守策略。
3.  **清晰的错误**：当用户错误使用API时（如对结构体使用默认比较），应在编译期就产生清晰的错误，而不是导致难以调试的运行时问题。
4.  **不牺牲性能**：通过编译期代码生成和内联，确保最终生成的代码性能与手写版本相当。

5.  **明确的文档**：清晰地记录每个API的用途、限制和使用方法，是库不可或缺的一部分。
