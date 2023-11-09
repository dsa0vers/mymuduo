#pragma once

#include <algorithm>
#include <string>
#include <vector>

// 网络库底层的缓冲器类型定义
namespace mymuduo {
class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t InitialSize = kInitialSize)
      : buffer_(kCheapPrepend + InitialSize),
        readIndex_(kCheapPrepend),
        writeIndex_(kCheapPrepend) {}
  size_t readableBytes() const { return writeIndex_ - readIndex_; }

  size_t writeableBytes() const { return buffer_.size() - writeIndex_; }

  size_t prependableBytes() const { return readIndex_; }

  // 返回缓冲区中可读数据的起始地址
  const char* peek() const { return begin() + readIndex_; }

  void retrieve(size_t len) {
    if (len < readableBytes()) {
      readIndex_ += len;
    } else {
      retrieveAll();
    }
  }

  void retrieveAll() { readIndex_ = writeIndex_ = kCheapPrepend; }

  // 把onMessage函数上报的Buffer数据，转成string类型的数据返回
  std::string retrieveAllAsString() {
    return retrieveAsString(readableBytes());  // 应用可读取数据的长度
  }

  std::string retrieveAsString(size_t len) {
    std::string result(peek(), len);
    retrieve(len);
    return result;
  }

  void ensureWriteableBytes(size_t len) {
    if (writeableBytes() < len) {
      makeSpace(len);  //扩容函数
    }
  }

  void append(const char* data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
    writeIndex_ += len;
  }

  char* beginWrite() { return begin() + writeIndex_; }

  const char* beginWrite() const { return begin() + writeIndex_; }

  // 通过fd读取数据
  ssize_t readFd(int fd, int* saveErrno);

  // 通过fd发送数据
  ssize_t writeFd(int fd, int* saveErrno);

 private:
  char* begin() { return &*buffer_.begin(); }

  const char* begin() const { return &*buffer_.begin(); }

  void makeSpace(size_t len) {
    if (writeableBytes() + prependableBytes() < len + kCheapPrepend) {
      buffer_.resize(writeIndex_ + len);
    } else {
      size_t readable = readableBytes();
      std::copy(begin() + readIndex_, begin() + writeIndex_,
                begin() + kCheapPrepend);
      readIndex_ = kCheapPrepend;
      writeIndex_ = readIndex_ + readable;
    }
  }

  std::vector<char> buffer_;
  size_t readIndex_;
  size_t writeIndex_;
};
}  // namespace mymuduo
