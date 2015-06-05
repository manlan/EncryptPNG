#pragma once

#include <array>
#include <sstream>
#include <Struct.h>
#include "cryptopp/osrng.h"  
#include "cryptopp/modes.h"
using namespace CryptoPP;

/**
 * 从流中读取一些数据
 */
template <int _Value, typename _Stream>
static std::array<char, _Value> ReadSome(_Stream &stream)
{
	std::array<char, _Value> buffer;
	for (unsigned int i = 0; i < _Value; ++i) buffer[i] = stream.get();
	return buffer;
}

/**
 * 从流中读取大量数据
 */
template <typename _Stream>
static std::stringstream ReadLarge(_Stream &stream, const int readsize)
{
	std::stringstream ss;
	for (int i = 0; i < readsize; ++i) ss.put(stream.get());
	return ss;
}

/**
 * 将数据写到流中
 */
template <typename _Stream>
static void WriteToSteam(const void *data, uint32 size, _Stream &stream)
{
	unsigned char *p = reinterpret_cast<unsigned char *>(const_cast<void *>(data));
	for (unsigned int i = 0; i < size; ++i) stream.put(p[i]);
}

/**
 * 移动流数据到另一个流
 */
template <typename _Source, typename _Target>
static void StreamMove(_Target &target, _Source &source, const uint32 size)
{
	for (uint64 i = 0; i < size; ++i) target.put(source.get());
}

/**
 * 信息加密
 */
void EncryptFileInfo(std::stringstream &ss, const std::array<unsigned char, KEY_SIZE> &key)
{
	std::string out;
	CTR_Mode<AES>::Encryption  Encryptor2(&key[0], KEY_SIZE, &key[0]);
	StringSource(ss.str(),
				 true,
				 new StreamTransformationFilter(Encryptor2,
				 new StringSink(out),
				 BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING,
				 true)
				 );
	ss.str(std::move(out));
}

/**
 * 信息解密
 */
static void DecryptFileInfo(std::stringstream &ss, const std::array<unsigned char, KEY_SIZE> &key)
{
	std::string out;
	CTR_Mode<AES>::Decryption Decryptor(&key[0], KEY_SIZE, &key[0]);

	StringSource(ss.str(),
				 true,
				 new StreamTransformationFilter(Decryptor,
				 new StringSink(out),
				 BlockPaddingSchemeDef::BlockPaddingScheme::NO_PADDING,
				 true)
				 );
	ss.str(std::move(out));
}