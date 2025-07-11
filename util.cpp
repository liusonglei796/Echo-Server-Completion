#include "util.h"  // 包含自定义工具头文件
#include <cstdio>  // 包含标准输入输出函数

// 错误检查函数实现：如果条件为真则抛出SyscallException异常
void errif(bool condition, const char *errmsg){
    if(condition){  // 检查条件是否为真
        throw SyscallException(errmsg);  // 抛出系统调用异常，包含错误消息
    }
}