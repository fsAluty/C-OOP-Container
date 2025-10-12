#include <stdio.h>
#include <string.h>
#include "vector.h"
#include "hashmap.h"

/*******************************************************************************
 *
 *  C-OOP-Container 最终演示
 *
 *  本示例展示了如何遵循“最佳实践”来使用本容器库，特别是如何处理
 *  自定义结构体和嵌套容器的场景。
 *
 *  核心原则：对于任何非单一名词的基本类型（如 `unsigned int`）或
 *            复杂类型（如指针 `char*`、结构体、嵌套容器），
 *            始终先使用 `typedef` 创建一个简洁的别名。
 *
 ******************************************************************************/


// --- 1. 定义所有需要用到的类型的别名 ---

// a. 为我们自己的学生结构体定义类型
typedef struct {
    int id;
    const char* name;
} Student;

// b. 为字符串指针定义一个清晰的别名
typedef const char* cstr;


// --- 2. 为自定义类型提供行为函数 ---

// a. 如何比较两个 Student 结构体是否相等
bool student_equals(Student s1, Student s2) {
    return s1.id == s2.id; // 在这个例子中，我们简化为只比较ID
}

// b. 如何在流中打印一个 Student 结构体
void student_display(FILE* stream, Student s) {
    fprintf(stream, "Student{id: %d, name: \"%s\"}", s.id, s.name);
}


// --- 3. 使用别名 "实例化" 我们需要的所有容器类型 ---

// b. 定义一个能存储 cstr (const char*) 的 vector 类型 (用于基本演示)
VECTOR_DEFINE(cstr);

// a. 定义一个能存储 Student 的 vector 类型
//    由于 Student 是自定义结构体，我们需要使用 _CUSTOM 宏
VECTOR_DEFINE_CUSTOM(Student, student_equals, student_display);


// c. 【关键步骤】为“学生的动态数组”这个嵌套类型定义一个别名
//    我们先在脑中想好，我们需要一个 `vector(Student)` 类型
//    所以我们为它创建一个别名 `StudentVec`

typedef vector(Student) StudentVec;

// d. 【关键步骤】定义一个键为 cstr、值为 StudentVec 的哈希表类型
//    现在，所有的类型名都是简单的单一名词，宏可以完美处理！
HASHMAP_DEFINE(cstr, StudentVec);


// --- 4. 主函数 - 演示用法 ---

void basic_demo() {
    printf("--- 基础用法演示 ---\n");
    
    vector(cstr) fruit_vec = vector_new(cstr);
    vector_push(fruit_vec, "Apple");
    vector_push(fruit_vec, "Banana");
    vector_push(fruit_vec, "Orange");

    printf("水果列表: ");
    vector_display(fruit_vec, stdout); // 输出: ["Apple", "Banana", "Orange"]
    printf("\n");
    
    vector_free(fruit_vec);
    printf("\n");
}

void nested_demo() {
    printf("--- 嵌套容器用法演示 (学生管理系统) ---\n");

    // 创建一个“班级花名册”哈希表
    // Key: cstr (班级名), Value: StudentVec (学生列表)
    hashmap(cstr, StudentVec) school_roster = hashmap_new(cstr, StudentVec);

    // --- 创建 "Class A" ---
    // 注意，我们现在使用 StudentVec 这个类型，而不是 vector(Student)
    StudentVec class_a = vector_new(Student);
    vector_push(class_a, ((Student){101, "Alice"}));
    vector_push(class_a, (Student){102, "Bob"});
    hashmap_put(school_roster, "Class A", class_a);

    // --- 创建 "Class B" ---
    StudentVec class_b = vector_new(Student);
    vector_push(class_b, (Student){201, "Charlie"});
    vector_push(class_b, (Student){202, "David"});
    hashmap_put(school_roster, "Class B", class_b);

    // --- 演示查询和打印 ---
    printf("学校所有班级花名册 (默认打印): \n");
    // hashmap_display 默认会打印 StudentVec 的地址，因为 StudentVec 是个指针
    hashmap_display(school_roster, stdout);
    printf("\n\n");

    // 查询 "Class A" 的学生列表
    const StudentVec* class_a_ptr = hashmap_get(school_roster, "Class A");
    if (class_a_ptr) {
        printf("查询 'Class A' 的学生: ");
        // *class_a_ptr 的类型是 StudentVec，即 vector(Student)
        // 它的 display 方法会使用我们为 Student 定制的 student_display 函数
        vector_display(*class_a_ptr, stdout);
        printf("\n");
    }
    printf("\n");

    // --- 清理内存（非常重要！） ---
    printf("开始清理内存...\n");
    hashmap_iterator(cstr, StudentVec) it = hashmap_get_iterator(school_roster);
    while (hashmap_iterator_next(it)) {
        const cstr* class_name = hashmap_iterator_current_key(it);
        const StudentVec* vec_to_free = hashmap_iterator_current_value(it);
        printf("释放班级 '%s' 的学生列表...\n", *class_name);
        vector_free(*vec_to_free); // 释放内部的 vector
    }
    // 最后释放哈希表本身
    hashmap_free(school_roster);
    printf("清理完成。\n");
}


int main() {
    basic_demo();
    nested_demo();
    return 0;
}