#ifndef _SECNET_H_INCLUDED
#define _SECNET_H_INCLUDED

#include <string>
#include <openssl/ssl.h>
#include <stdint.h>

#include "sigslot.h"

class SecNet
{
	public:
		typedef struct
		{
			uint8_t version;
			uint8_t command;
			uint64_t payloadLength;
		} __attribute__((packed)) Packet;

		static const uint16_t DefaultPort = 3938;

		static void Initialize(std::string listen, std::string certificateFile, std::string privateKeyFile, std::string dhParamFile, std::string tlsCipherList = "");

		static int sslErrorCallback(const char* message, size_t length, void* userData);

		virtual ~SecNet();

		static sigslot::signal3<Packet, uint8_t*, SecNet*> receivedPacket;
		void sendPacket(Packet packet, uint8_t* payload);

	private:
		typedef struct
		{
			uint16_t opcode: 4;
			uint16_t rsvd : 3;
			uint16_t fin : 1;
			uint16_t payloadLen : 7;
			uint16_t maskFlag : 1;
		} WebSocketFrameHeader;

		static bool _listening;
		static  int _serverSocket;

		static void ListenLoop(SSL_CTX* serverContext);

		SecNet(int socket, SSL_CTX* serverContext);

		void handleWebSocket(Packet* packet);
		void handleWebSocketFrame();

		SSL* _ssl;
		int _socket;
};

#endif
