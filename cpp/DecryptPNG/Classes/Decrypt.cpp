#include "Decrypt.h"

#include <fstream>
#include <iostream>
#include "Tools.h"
#include "Files.h"


// 解密PNG图片
void DecryptPNG(const std::vector<std::string> &filelist, const std::array<unsigned char, KEY_SIZE> &key)
{
	for (auto &filename : filelist)
	{
		std::ifstream in_file(filename, std::ios::binary | std::ios::ate);
		if (!in_file.is_open())
		{
			std::cerr << "打开" << filename << " 失败！" << std::endl;
			return;
		}

		// 读取文件信息位置
		uint64 end_pos = in_file.tellg();
		in_file.seekg(end_pos - sizeof(uint64));
		uint64 block_start_pos = *reinterpret_cast<uint64 *>(&(ReadSome<sizeof(uint64)>(in_file)[0]));
		in_file.seekg(block_start_pos);

		// 解密文件信息
		auto block_info = ReadLarge(in_file, uint32(end_pos - sizeof(uint64) - block_start_pos));
		std::string sssaasd = block_info.str();
		DecryptBlockInfo(block_info, key);

		// 验证文件信息
		auto infohead = ReadSome<sizeof(BLOCK_HEAD)>(block_info);
		for (unsigned int i = 0; i < infohead.size(); ++i)
		{
			if (infohead[i] != BLOCK_HEAD[i])
			{
				std::cerr << "密钥错误，解密" << filename << " 失败！" << std::endl;
				continue;
			}
		}

		std::ofstream out_file(path::splitext(filename)[0] + ".png", std::ios::binary);
		if (!out_file.is_open())
		{
			std::cerr << "创建" << path::splitext(filename)[1] << ".png" << " 失败！" << std::endl;
			continue;
		}

		// 写入文件头
		WriteToSteam(HEAD_DATA, sizeof(HEAD_DATA), out_file);

		// 读取数据块
		uint64 read_size = 0;
		while (true)
		{
			// 读取数据块信息
			Block block;
			memcpy(&block, &ReadSome<sizeof(Block)>(block_info)[0], sizeof(Block));
			if (block_info.peek() == block_info.eofbit)
			{
				out_file.clear();
				std::cerr << "the %s file format error!" << std::endl;
				break;
			}

			// 写入数据块长度
			char reverse_size[sizeof(block.size)];
			memcpy(reverse_size, &block.size, sizeof(reverse_size));
			std::reverse(reverse_size, reverse_size + sizeof(reverse_size));
			WriteToSteam(reverse_size, sizeof(reverse_size), out_file);

			// 写入数据块名称
			WriteToSteam(&block.name, sizeof(block.name), out_file);

			// 写入数据块内容
			std::string s_name(block.name, sizeof(block.name));
			if (strcmp(s_name.c_str(), "IHDR") == 0)
			{
				IHDRBlock ihdr;
				memcpy(&ihdr, &block, sizeof(Block));
				memcpy(((char *)&ihdr) + sizeof(Block), &ReadSome<sizeof(IHDRBlock) - sizeof(Block)>(block_info)[0], sizeof(IHDRBlock) - sizeof(Block));
				WriteToSteam(ihdr.data, sizeof(ihdr.data), out_file);
			}
			else if (strcmp(s_name.c_str(), "IEND") == 0)
			{
				WriteToSteam(IEND_DATA, sizeof(IEND_DATA), out_file);
				std::cout << "成功解密：" << filename << std::endl;
				break;
			}
			else
			{
				in_file.seekg(block.pos - read_size);
				StreamMove(out_file, in_file, block.size + CRC_SIZE);
				read_size += block.size + CRC_SIZE;
			}
		}
	}
}