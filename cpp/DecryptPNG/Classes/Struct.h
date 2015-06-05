#pragma once

#include "Types.h"

static const uint32 KEY_SIZE = 16;

static const uint32 HEAD_SIZE = 8;

static const uint32 CRC_SIZE = 4;

static const unsigned char HEAD_DATA[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

static const unsigned char IEND_DATA[] = { 0x49, 0x45, 0x47, 0x44, 0xae, 0x42, 0x60, 0x82 };

static const unsigned char BLOCK_HEAD[] = { 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x50, 0x4e, 0x47 };

#pragma pack(push, 1)

struct Block
{
	char name[4];
	uint64 pos;
	uint32 size;
};

struct IHDRBlock
{
	Block block;
	char data[13 + CRC_SIZE];
};

#pragma pack(pop)