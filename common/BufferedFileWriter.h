//
// Created by Yi Lu on 3/21/19.
//

#pragma once

#include <glog/logging.h>

#include <cstring>
#include <fcntl.h>
#include <stdio.h>

class BufferedFileWriter {

public:
  BufferedFileWriter(const char *filename) {
    fd = open(filename, O_WRONLY | O_CREAT,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);//只写，不存在就创建，文件权限，允许用户、组和其他用户读写文件。
    CHECK(fd >= 0);
    bytes_total = 0;
  }

  void write(const char *str, long size) {

    if (bytes_total + size < BUFFER_SIZE) {
      memcpy(buffer + bytes_total, str, size);
      bytes_total += size;
      return;
    }
    //缓冲区即将满，计算剩余空间，将缓冲区数据写入文件，然后将剩余数据写入缓冲区
    auto copy_size = BUFFER_SIZE - bytes_total;

    memcpy(buffer + bytes_total, str, copy_size);
    bytes_total += copy_size;
    flush();
    //处理剩余数据
    str += copy_size;
    size -= copy_size;

    if (size >= BUFFER_SIZE) {
      int err = ::write(fd, str, size);
      CHECK(err >= 0);
      bytes_total = 0;
    } else {
      memcpy(buffer, str, size);
      bytes_total += size;
    }
  }

  void flush() {
    DCHECK(fd >= 0);
    if (bytes_total > 0) {
      int err = ::write(fd, buffer, bytes_total);
      CHECK(err >= 0);
    }
    bytes_total = 0;
  }

  void close() {
    flush();
    int err = ::close(fd);
    CHECK(err == 0);
  }

public:
  static constexpr uint32_t BUFFER_SIZE = 1024 * 1024 * 4; // 4MB

private:
  int fd;
  char buffer[BUFFER_SIZE];
  std::size_t bytes_total;
};