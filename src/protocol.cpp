#include <iostream>

#include "../include/protocol.h"

Protocol::Protocol(Blockstorage* storage)
{
	_blockStorage = storage;
}

void Protocol::packetReceived(SecNet::Packet packet, uint8_t* payload, SecNet* secnet)
{
	// sanity-checks
	if(packet.version != ((VERSION_MAJOR << 4) | VERSION_MINOR))
	{
		std::cout << "Unsupported Packet-version: " << std::hex << *(uint64_t*)&packet << std::endl;
		exit(-1); // we don't like non-conforming clients
	}

	switch(packet.command)
	{
		case COMMAND_READ_BLOCK:
		{
			if(packet.payloadLength != 8)
				exit(-1);
			
			uint64_t blockNumber = extractUint64FromPayload(payload);
			uint8_t* blockData = _blockStorage->getBlock(blockNumber);
			
			SecNet::Packet answer;
			answer.version = packet.version;
			answer.payloadLength = BLOCK_SIZE;
			answer.command = COMMAND_BLOCK_ANSWER;

			secnet->sendPacket(answer, blockData);
			break;
		}
		case COMMAND_WRITE_BLOCK:
		{
			if(packet.payloadLength != 8 + BLOCK_SIZE)
				exit(-1);

			uint64_t blockNumber = extractUint64FromPayload(payload);
			_blockStorage->storeBlock(blockNumber, (uint8_t*)(payload + 8));

			SecNet::Packet answer;
			answer.version = packet.version;
			answer.payloadLength = 0;
			answer.command = COMMAND_OK;

			secnet->sendPacket(answer, NULL);

			break;
		}
		case COMMAND_LOCK_BLOCKS:
			break;
		case COMMAND_GET_MAPPING:
			break;
		case COMMAND_IS_BLOCK_AVAILABLE:
		{
			uint64_t blockNumber = extractUint64FromPayload(payload);
			bool available = _blockStorage->isBlockAvailable(blockNumber);

			SecNet::Packet answer;
			answer.version = packet.version;
			answer.payloadLength = 0;
			answer.command = available ? COMMAND_OK : COMMAND_NOK;

			secnet->sendPacket(answer, NULL);

			break;
		}
		default:
			exit(-1);
	}
}

inline uint64_t Protocol::extractUint64FromPayload(uint8_t* payload)
{
	uint64_t result;
	result = (uint64_t)payload[0] << 56;
	result |= (uint64_t)payload[1] << 48;
	result |= (uint64_t)payload [2] << 40;
	result |= (uint64_t)payload[3] << 32;
	result |= (uint64_t)payload[4] << 24;
	result |= (uint64_t)payload[5] << 16;
	result |= (uint64_t)payload[6] << 8;
	result |= (uint64_t)payload[7];

	return result;
}
