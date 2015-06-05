#pragma once


#include <array>
#include <string>
#include <vector>
#include "Struct.h"

/**
 * 解密PNG图片
 * @param filelist 文件列表
 */
void DecryptPNG(const std::vector<std::string> &filelist, const std::array<unsigned char, KEY_SIZE> &key);