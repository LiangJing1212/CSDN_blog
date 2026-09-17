#include <cstddef>
#include <cstring>
#include <iostream>

// 一个持有堆内存的缓冲区，用来观察拷贝 vs 移动的真实代价
class Buffer {
public:
  explicit Buffer(std::size_t n) : size_(n), data_(new char[n]) {
    std::cout << "  [ctor]  alloc " << n << " bytes\n";
  }

  // 拷贝构造：深拷贝
  Buffer(const Buffer & other) : size_(other.size_), data_(new char[other.size_]) {
    std::memcpy(data_, other.data_, size_);
    std::cout << "  [copy]  deep copy " << size_ << " bytes\n";
  }

  // 移动构造：偷走资源
  Buffer(Buffer && other) noexcept
  : size_(other.size_), data_(other.data_) {
    other.size_ = 0;
    other.data_ = nullptr;
    std::cout << "  [move]  steal ptr, no alloc\n";
  }

  ~Buffer() {
    delete[] data_;
    if (data_) { std::cout << "  [dtor]  free\n"; }
  }

private:
  std::size_t size_ = 0;
  char * data_ = nullptr;
};

int main() {
  std::cout << "== 拷贝路径 ==\n";
  Buffer a(1024 * 1024);          // 分配 1MB
  Buffer b(a);                    // 拷贝 → 又分配 1MB

  std::cout << "\n== 移动路径 ==\n";
  Buffer c(1024 * 1024);          // 分配 1MB
  Buffer d(std::move(c));         // 移动 → 0 分配，直接偷指针
}
