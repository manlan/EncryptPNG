#pragma once

#include <array>
#include <string>
#include <vector>
#include "Struct.h"

/**
 * 加密PNG图片
 * @param filelist 文件刘表
 */
void EncryptPNG(const std::vector<std::string> &filelist, const std::array<unsigned char, KEY_SIZE> &key);