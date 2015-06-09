#pragma once

#include <array>
#include <vector>
#include "CCData.h"

namespace cocos2d
{
	/**
	 * 分解文件名的扩展名
	 * @param file_path 文件名
	 */
	std::array<std::string, 2> splitext(const std::string &file_path);
	
	/**
	 * 解密图片
	 * @param filename 文件名称
	 * @param data 文件数据
	 */
	std::vector<unsigned char> DecryptImage(const std::string &filename, Data &data);
}