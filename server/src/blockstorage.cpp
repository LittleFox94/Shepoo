#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <strings.h>

#include "../include/blockstorage.h"

Blockstorage::Blockstorage(std::string path)
{
	_path = path;

	struct stat sb;
	bzero(&sb, sizeof(sb));
	stat(_path.c_str(), &sb);

	_fileLength = sb.st_size;
}

Blockstorage::~Blockstorage()
{
}

uint8_t* Blockstorage::getBlock(uint64_t num)
{
	if(!isBlockAvailable(num))
		return NULL;

	int fd = open(_path.c_str(), O_LARGEFILE);

	lseek64(fd, num * BLOCK_SIZE, SEEK_SET);
	lockf(fd, F_LOCK, BLOCK_SIZE);

	uint8_t* block = new uint8_t[BLOCK_SIZE];
	read(fd, block, BLOCK_SIZE);

	lseek64(fd, num * BLOCK_SIZE, SEEK_SET);
	lockf(fd, F_ULOCK, BLOCK_SIZE);
	close(fd);

	return block;
}

void Blockstorage::storeBlock(uint64_t num, uint8_t* data)
{
	
}

bool Blockstorage::isBlockAvailable(uint64_t num)
{
	return (num * BLOCK_SIZE) <= (_fileLength - BLOCK_SIZE); 
}
