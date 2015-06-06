#include <iostream>
#include "Decrypt.h"


int main(int argc, char *argv[])
{
	aes_key key = { 0 };
	std::cout << "请输入16位密钥：" << std::endl;
	std::cin.get((char *)&key[0], key.size());

	std::vector<std::string> files;
	for (int i = 1; i < argc; ++i)
	{
		files.push_back(argv[i]);
	}
	DecryptPNG(files, key);
	
	return 0;
}