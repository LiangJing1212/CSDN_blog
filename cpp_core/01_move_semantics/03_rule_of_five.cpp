#include <iostream>
#include <memory>
#include <vector>

// ============ Rule of Zero：全用 RAII 成员，五样都不写 ============
class ImageZero {
public:
  explicit ImageZero(std::size_t n) : data_(new uint8_t[n]) {}
  // 编译器自动生成：析构、拷贝、移动，全部正确（unique_ptr 禁拷贝只允许移动）
private:
  std::unique_ptr<uint8_t[]> data_;
};

// ============ Rule of Five：手管裸资源，五样全写 ============
class BufferFive {
public:
  explicit BufferFive(std::size_t n)
  : size_(n), data_(new uint8_t[n]) {}

  ~BufferFive() { delete[] data_; }                  // 1 析构

  BufferFive(const BufferFive & other)               // 2 拷贝构造
  : size_(other.size_), data_(new uint8_t[other.size_]) {
    std::copy(other.data_, other.data_ + size_, data_);
  }

  BufferFive & operator=(const BufferFive & other) { // 3 拷贝赋值
    if (this != &other) {
      delete[] data_;                                // 先释放旧资源
      size_ = other.size_;
      data_ = new uint8_t[size_];
      std::copy(other.data_, other.data_ + size_, data_);
    }
    return *this;
  }

  BufferFive(BufferFive && other) noexcept           // 4 移动构造
  : size_(other.size_), data_(other.data_) {
    other.size_ = 0;
    other.data_ = nullptr;
  }

  BufferFive & operator=(BufferFive && other) noexcept { // 5 移动赋值
    if (this != &other) {
      delete[] data_;
      size_ = other.size_;
      data_ = other.data_;
      other.size_ = 0;
      other.data_ = nullptr;
    }
    return *this;
  }

  std::size_t size() const { return size_; }

private:
  std::size_t size_ = 0;
  uint8_t * data_ = nullptr;
};

// ============ 写一半的坑：只有析构，没有移动 ============
class HalfWay {
public:
  explicit HalfWay(std::size_t n) : size_(n), data_(new uint8_t[n]) {}
  ~HalfWay() { delete[] data_; }
  // 没写移动构造 → 编译器不生成移动 → 但拷贝构造仍隐式生成
  // 结果：临时对象按值传递时【静默走深拷贝】，不报错但性能回退
  // 这正是"五要一起写"的原因（或干脆 Rule of Zero）
private:
  std::size_t size_ = 0;
  uint8_t * data_ = nullptr;
};

int main() {
  // Rule of Zero：正确 + 简洁
  std::vector<ImageZero> vec0;
  vec0.push_back(ImageZero(1024));  // unique_ptr 可移动，OK

  // Rule of Five：手动管理，行为正确
  BufferFive a(1024);
  BufferFive b = a;              // 拷贝构造 → 深拷贝
  BufferFive c = std::move(a);   // 移动构造 → 掏空 a
  std::cout << "b.size()=" << b.size() << "  c.size()=" << c.size()
            << "  a.size()=" << a.size() << " (moved-from)\n";

  // HalfWay 在 vector 按值存时会怎样？取消注释，它【能编译】但走深拷贝
  // std::vector<HalfWay> vec;
  // vec.push_back(HalfWay(1024));  // ← 静默深拷贝，无报错
}
