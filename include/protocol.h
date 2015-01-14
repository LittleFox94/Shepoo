#ifndef _PROTOCOL_H_INCLUDED
#define _PROTOCOL_H_INCLUDED

#include <array>
#include <cstdint>

#include "secnet.h"
#include "sigslot.h"

class Protocol
{
	public:
		Protocol(SecNet* network);
		virtual ~Protocol();

		void sendBlock(std::array<uint8_t> data);
		void sendMapping(uint64_t blockA, uint64_t blockB);

		signal1<uint64_t> signalBlockRequested;
		signal1<uint64_t> mappingRequested;
};

#endif
