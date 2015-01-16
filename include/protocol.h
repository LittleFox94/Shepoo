#ifndef _PROTOCOL_H_INCLUDED
#define _PROTOCOL_H_INCLUDED

#include <stdint.h>

#include "secnet.h"
#include "sigslot.h"
#include "blockstorage.h"

class Protocol : sigslot::has_slots<>
{
	enum Commands
	{
		COMMAND_READ_BLOCK = 1,
		COMMAND_WRITE_BLOCK = 2,
		COMMAND_LOCK_BLOCKS = 3,
		COMMAND_GET_MAPPING = 4,
		COMMAND_BLOCK_ANSWER = 5,
		COMMAND_IS_BLOCK_AVAILABLE = 6,
		COMMAND_OK = 7,
		COMMAND_NOK = 8
	};

	public:
		Protocol(Blockstorage* storage);

		void packetReceived(SecNet::Packet packet, uint8_t* payload, SecNet* secnet);

	private:
		Blockstorage* _blockStorage;

		static inline uint64_t extractUint64FromPayload(uint8_t* payload);
};

#endif
