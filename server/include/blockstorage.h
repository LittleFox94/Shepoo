#ifndef _BLOCKSTORAGE_H_INCLUDED
#define _BLOCKSTORAGE_H_INCLUDED

#include <string>
#include <stdint.h>

class Blockstorage
{
	public:
		Blockstorage(std::string path);
		virtual ~Blockstorage();

		uint8_t* getBlock(uint64_t num);
		void storeBlock(uint64_t num, uint8_t* data);
		bool isBlockAvailable(uint64_t num);

	private:
		std::string _path;
		uint64_t _fileLength;
};

#endif
