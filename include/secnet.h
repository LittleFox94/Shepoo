#ifndef _SECNET_H_INCLUDED
#define _SECNET_H_INCLUDED

#include <string>
#include <openssl/ssl.h>

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

		signal1<Packet*> receivedPacket;
		void sendPacket(Packet* packet);

	private:
		bool _listening;
		int _serverSocket;

		static void ListenLoop(SSL_CTX* serverContext);
};

#endif
