#ifndef _NETWORK_H_INCLUDED
#define _NETWORK_H_INCLUDED

#include <array>
#include <cstdint>

#include "secnet.hpp"
#include "sigslot.h"

#define NETWORK_PORTNUMBER	3938

class Network
{
	public:
		Network(uint16_t port = NETWORK_PORTNUMBER);
		virtual ~Network();

		void sendBlock(std::array<uint8_t> data);
		void sendMapping(uint64_t blockA, uint64_t blockB);

		signal1<uint64_t> signalBlockRequested;
		signal1<uint64_t> mappingRequested;
};

#endif
