#ifndef _BLOCKSTORAGE_H_INCLUDED
#define _BLOCKSTORAGE_H_INCLUDED

#include <string>
#include <array>

#define BLOCKSIZE	4096

class Blockstorage
{
	public:
		Blockstorage(std::string path);
		virtual ~Blockstorage();

		std::array<uint8_t> getBlock(uint64_t num);
		void storeBlock(uint64_t num, std::array<uint8_t> data);
		bool isBlockAvailable(uint64_t num);
};

#endif
