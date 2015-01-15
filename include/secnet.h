#ifndef _SECNET_H_INCLUDED
#define _SECNET_H_INCLUDED

#include <string>
#include <openssl/ssl.h>

#include "sigslot.h"

class SecNet
{
	typedef struct
	{
		uint8_t version;
		uint8_t command;
		uint64_t payloadLength;
		uint8_t* payload;
	} __attribute__((packed)) Packet;

	public:
		static const uint16_t DefaultPort = 3938;

		static void Initialize(std::string listen, std::string certificateFile, std::string privateKeyFile, std::string dhParamFile, std::string tlsCipherList = "");

		SecNet(int socket);
		virtual ~SecNet();

		sigslot::signal1<Packet*> receivedPacket;
		void sendPacket(Packet* packet);

	private:
		static bool _listening;
		static  int _serverSocket;

		static void ListenLoop(SSL_CTX* serverContext);
};

#endif
