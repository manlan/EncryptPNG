#include "Encrypt.h"

#include <fstream>
#include <iostream>
#include "Tools.h"
#include "Files.h"

/**
 * 写入单个文件数据
 * @param filename 文件名称
 * @param outstream 输出文件流
 * @param blockinfo 输出块信息
 */
void WriteFileData(const std::string &filename, std::ofstream &outstream, std::stringstream &blockinfo)
{
	std::ifstream file;
	file.open(filename, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "打开" << filename << " 失败！" << std::endl;
		return;
	}

	// 读取文件头
	auto head = ReadSome<HEAD_SIZE>(file);

	// 读取数据块
	while (true)
	{
		Block block;

		// 获取数据块长度
		auto lenght = ReadSome<4>(file);
		if (file.eof()) break;
		std::reverse(lenght.begin(), lenght.end());
		auto block_size = *reinterpret_cast<int *>(&lenght[0]);

		// 获取数据块名称
		auto block_name = &(ReadSome<4>(file)[0]);

		// 获取数据块内容
		auto block_data = ReadLarge(file, block_size + CRC_SIZE);

		// 数据块信息
		block.size = block_size;
		block.pos = outstream.tellp();
		memcpy(block.name, &block_name[0], sizeof(block.name));

		// 根据数据类型进行处理
		std::string s_name(block.name, sizeof(block.name));
		if (strcmp(s_name.c_str(), "IHDR") == 0)
		{
			IHDRBlock ihdr;
			ihdr.block = block;
			memcpy(ihdr.data, block_data.str().c_str(), sizeof(ihdr.data));
			char *user_data = reinterpret_cast<char *>(&ihdr);
			for (unsigned int i = 0; i < sizeof(IHDRBlock); ++i) blockinfo.put(user_data[i]);
		}
		else if (strcmp(s_name.c_str(), "IEND") == 0)
		{
			char *user_data = reinterpret_cast<char *>(&block);
			for (unsigned int i = 0; i < sizeof(Block); ++i) blockinfo.put(user_data[i]);
		}
		else if (strcmp(s_name.c_str(), "PLTE") == 0 || strcmp(s_name.c_str(), "IDAT") == 0)
		{
			char *user_data = reinterpret_cast<char *>(&block);
			for (int i = 0; i < sizeof(Block); ++i) blockinfo.put(user_data[i]);
			for (unsigned int i = 0; i < block_size + CRC_SIZE; ++i) outstream.put(block_data.get());
		}
	}
}

// 加密PNG图片
void EncryptPNG(const std::vector<std::string> &filelist, const std::array<unsigned char, KEY_SIZE> &key)
{
	std::ofstream out_file;
	std::stringstream block_info;

	for (auto &filename : filelist)
	{
		// 文件信息头部
		for (auto ch : BLOCK_HEAD) block_info.put(ch);

		// 写入文件数据
		std::string out_path = path::splitext(filename)[0] + ".epng";
		out_file.open(out_path, std::ios::binary);
		if (!out_file.is_open())
		{
			std::cerr << "创建" << filename << " 失败！" << std::endl;
			out_file.close();
			block_info.str("");
			continue;
		}
		WriteFileData(filename, out_file, block_info);

		// 记录信息位置
		uint64 pos = out_file.tellp();
		char *user_data = reinterpret_cast<char *>(&pos);

		// 文件信息加密
		EncryptBlockInfo(block_info, key);

		// 写入文件信息
		StreamMove(out_file, block_info, block_info.str().size());
		for (unsigned int i = 0; i < sizeof(uint64); ++i) out_file.put(user_data[i]);

		std::cout << "已生成：" << out_path.c_str() << std::endl;

		out_file.close();
		block_info.str("");
	}
}