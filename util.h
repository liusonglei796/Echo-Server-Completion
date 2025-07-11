#ifndef UTIL_H  // 防止头文件重复包含的宏定义
#define UTIL_H

#include <stdexcept>  // 包含标准异常类定义
#include <cstring>    // 包含C字符串操作函数

// 自定义系统调用异常类，继承自标准运行时异常
class SyscallException : public std::runtime_error {
public: explicit
    // 构造函数，接受错误消息字符串
    SyscallException(const char* msg) : std::runtime_error(msg) {}
};

// 错误检查函数声明：如果条件为真则抛出异常
void errif(bool condition, const char* errmsg);

#endif // UTIL_H  // 头文件保护结束