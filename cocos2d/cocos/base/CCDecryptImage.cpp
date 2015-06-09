#include "CCDecryptImage.h"

#include <sstream>
#include "CCAES.h"
#include "ccMacros.h"

namespace cocos2d
{
	/* crc码长度 */
	static const uint32_t CRC_SIZE = 4;

	/* png文件头部 */
	static const unsigned char HEAD_DATA[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

	/* png尾部crc码 */
	static const unsigned char IEND_DATA[] = { 0xae, 0x42, 0x60, 0x82 };

	/* 数据块头部（用于验证解密是否成功） */
	static const unsigned char BLOCK_HEAD[] = { 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x50, 0x4e, 0x47 };

	/* 默认密钥 */
	static const aes_key DEAULT_KEY = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 };

#pragma pack(push, 1)

	struct Block
	{
		char name[4];
		uint64_t pos;
		uint32_t size;
	};

	struct IHDRBlock
	{
		Block block;
		char data[13 + CRC_SIZE];
	};

#pragma pack(pop)

	std::array<std::string, 2> splitext(const std::string &file_path)
	{
		std::string::size_type pos = file_path.rfind('.');
		std::array<std::string, 2> text;
		if (std::string::npos != pos)
		{
			text[1] = file_path.substr(pos);
			text[0] = file_path.substr(0, pos);
		}
		else
		{
			text[0] = file_path;
		}
		return text;
	}

	template <int _Value, typename _Stream>
	std::array<char, _Value> ReadSome(_Stream &stream)
	{
		std::array<char, _Value> buffer;
		for (unsigned int i = 0; i < _Value; ++i) buffer[i] = stream.get();
		return buffer;
	}

	void DecryptBlock(std::stringstream &ss, const aes_key &key)
	{
		const std::streamoff contents_size = ss.tellp() - ss.tellg();
		const uint32_t block_size = (uint32_t)(contents_size + AES_BLOCK_SIZE - contents_size % AES_BLOCK_SIZE);
		std::vector<uint8_t> buffer;
		buffer.resize(block_size);
		for (uint32_t i = 0; i < contents_size; ++i) buffer[i] = ss.get();
		AES::DecryptData(&buffer[0], block_size, key);
		ss.seekg(0); ss.seekp(0);
		for (uint32_t i = 0; i < block_size; ++i) ss.put(buffer[i]);
	}

	std::vector<unsigned char> DecryptImage(const std::string &filename, Data &data)
	{
		CCAssert(!data.isNull(), "data is null!");

		const uint64_t block_start_pos = *reinterpret_cast<uint64_t *>(data.getBytes() + data.getSize() - sizeof(uint64_t));

		std::stringstream block_info;
		for (uint64_t i = block_start_pos; i < data.getSize() - sizeof(uint64_t); ++i)
		{
			block_info.put(*(data.getBytes() + i));
		}

		DecryptBlock(block_info, DEAULT_KEY);

		auto block_head = ReadSome<sizeof(BLOCK_HEAD)>(block_info);
		for (unsigned int i = 0; i < block_head.size(); ++i)
		{
			if (block_head[i] != BLOCK_HEAD[i])
			{
				CCAssert(false, "the key is wrong!");
			}
		}

		std::vector<unsigned char> image_data;
		image_data.reserve(data.getSize());

		for (auto ch : HEAD_DATA) image_data.push_back(ch);

		while (true)
		{
			Block block;
			memcpy(&block, &(ReadSome<sizeof(Block)>(block_info)[0]), sizeof(Block));
			if (block_info.eof())
			{
				CCAssert(false, "");
				CCLOG("the %s file format error!", filename.c_str());
				break;
			}

			char reverse_size[sizeof(block.size)];
			memcpy(reverse_size, &block.size, sizeof(reverse_size));
			std::reverse(reverse_size, reverse_size + sizeof(reverse_size));

			for (auto ch : reverse_size) image_data.push_back(ch);
			for (auto ch : block.name) image_data.push_back(ch);

			std::string s_name(block.name, sizeof(block.name));
			if (strcmp(s_name.c_str(), "IHDR") == 0)
			{
				IHDRBlock ihdr;
				memcpy(&ihdr, &block, sizeof(Block));
				memcpy(((char *)&ihdr) + sizeof(Block), &ReadSome<sizeof(IHDRBlock) - sizeof(Block)>(block_info)[0], sizeof(IHDRBlock) - sizeof(Block));
				for (auto ch : ihdr.data) image_data.push_back(ch);
			}
			else if (strcmp(s_name.c_str(), "IEND") == 0)
			{
				for (auto ch : IEND_DATA) image_data.push_back(ch);
				CCLOG("decrypt %s success!", filename.c_str());
				break;
			}
			else
			{
				for (uint32_t i = 0; i < block.size + CRC_SIZE; ++i)
				{
					image_data.push_back(*(data.getBytes() + block.pos + i));
				}
			}
		}
		return image_data;
	}
}