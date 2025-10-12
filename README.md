# C-OOP-Container: C语言面向对象泛型容器库

## 概述

**C-OOP-Container** 是一个为C语言精心打造的、类型安全的、仅需头文件的泛型容器库。它以**面向对象（Object-Oriented Programming）**的设计思想为核心，提供了高性能的动态数组 (`vector`) 和哈希表 (`hashmap`)。

本项目旨在将现代编程语言的优雅与C语言的极致性能相结合，解决C语言在数据结构使用上的两大痛点：
1.  **缺乏泛型**：传统的C语言数据结构要么为每种类型手写一套实现，要么使用`void*`牺牲类型安全和性能。
2.  **API繁琐**：许多C库需要用户编写大量模板式的“胶水代码”才能使用。

**C-OOP-Container** 通过模拟面向对象的“类”与“实例”概念，并结合精巧的宏和现代C语言特性 (C11 `_Generic`)，实现了“开箱即用”的非凡体验。

## 核心特性：面向对象的设计哲学

本库最大的特色在于其**面向对象的实现方式**：

*   **模拟“类”与“方法”**：每个容器的定义都包含一个数据结构体和一个函数指针表（`fns`）。这就像一个C++的类，数据成员和成员方法被封装在一起。
*   **统一的调用接口**：所有操作都通过 `container->fns->method(...)` 的形式调用，这与 `object->method(...)` 的面向对象调用风格如出一辙，使得API既统一又直观。
*   **多态与可扩展性**：通过替换函数指针，可以轻松实现行为的定制。例如，为自定义结构体提供专用的`equals`, `hash`, `display`函数，就像在C++中重载操作符或实现虚函数一样。

### 其他关键特性

*   **真正的泛型**：通过编译期代码生成，为每种类型创建专门的、类型安全的数据结构，无`void*`带来的运行时开销。
*   **开箱即用**：内置对所有C语言基本算术类型、指针和字符串 (`const char*`) 的支持。无需为它们编写哈希、比较或打印函数。
*   **API简洁优雅**：提供了一套全小写的宏接口（如`vector_push`, `hashmap_get`），风格统一，简单易用。
*   **健壮的错误处理**：
    *   在编译期，对结构体等复杂类型使用默认的比较函数会直接报错，清晰地引导用户使用自定义函数。
    *   在运行时，对`hashmap`的容量进行检查，强制要求其为2的幂，确保哈希算法的高效性。
*   **清晰的文档**：所有公开的API宏都配有符合Doxygen规范的详细注释，解释了其功能、参数和使用限制。
*   **仅头文件**：整个库只由 `vector.h` 和 `hashmap.h` 两个文件组成，可以非常方便地集成到任何项目中。

## 快速上手

### Vector (动态数组)

```c
#include <stdio.h>
#include "vector.h"

// 为 "const char*" 创建一个单一名词的别名
typedef const char* cstr;

// 1. "实例化"一个整数vector的"类"定义
VECTOR_DEFINE(int);
// 2. "实例化"一个字符串vector的"类"定义
VECTOR_DEFINE(cstr);

int main() {
    // === 整数Vector示例 ===
    // 创建一个"对象"实例
    vector(int) int_vec = vector_new(int);
  
    // 调用"方法"
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
    vector_free(int_vec); // "销毁"对象


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

// 1. "实例化"一个键为字符串、值为整数的哈希表"类"定义
HASHMAP_DEFINE(cstr, int);

int main() {
    // 创建一个"对象"实例，并指定初始容量
    hashmap(cstr, int) map = hashmap_new_with_capacity(cstr, int, 8);

    // 调用"方法"
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

    hashmap_free(map); // "销毁"对象
    return 0;
}
```

## 设计哲学

**C-OOP-Container** 的设计遵循以下原则：
1.  **面向对象的思维**：将数据和操作封装在一起，通过统一的接口进行交互，是本库的最高设计准则。
2.  **简单优先**：提供简洁明了的API，隐藏内部实现的复杂性。
3.  **安全默认**：默认行为应尽可能安全。例如，对未知类型默认打印其地址而不是尝试解释其内容。
4.  **清晰的错误**：当用户错误使用API时（如对结构体使用默认比较），应在编译期就产生清晰的错误，而不是导致难以调试的运行时问题。
5.  **不牺牲性能**：通过编译期代码生成和内联，确保最终生成的代码性能与手写版本相当。
6.  **明确的文档**：清晰地记录每个API的用途、限制和使用方法，是库不可或缺的一部分。
