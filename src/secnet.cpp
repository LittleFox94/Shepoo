#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <resolv.h>

#include "../include/secnet.h"

// no sane human will use packets with more than 128MB payload
// at least not in the current century.
#define MAX_PACKET_PAYLOAD_SIZE	134217728

bool SecNet::_listening;
int SecNet::_serverSocket;
sigslot::signal3<SecNet::Packet, uint8_t*, SecNet*> SecNet::receivedPacket;

SecNet::SecNet(int socket, SSL_CTX* serverContext)
{
	close(SecNet::_serverSocket);
	
	_socket = socket;
	_ssl = SSL_new(serverContext);

	if(_ssl == 0)
	{
		std::cout << "Could not create new SSL connection!" << std::endl;
		exit(-1); // if the fork can't handle the client, there is no reason to allow it staying alive >:]
	}

	SSL_set_fd(_ssl, _socket);

	if(SSL_accept(_ssl) != 1)
	{
		std::cout << "SSL Handshake failed!" << std::endl;
		ERR_print_errors_cb(SecNet::sslErrorCallback, NULL);
		exit(-1); // still no reason to allow it staying alive >:]
	}

	// handle the connection
	Packet packet;
	bzero(&packet, sizeof(packet));
	SSL_read(_ssl, &packet, sizeof(packet));

	if(*(int*)&packet == 0x20544547) // don't hurt me .. it's just "GET " as integer
	{
		// meh .. It's HTTP, probably Websocket as we use this one ...
		handleWebSocket(&packet);
		return;
	}
	
	if(packet.payloadLength > MAX_PACKET_PAYLOAD_SIZE)
	{
		std::cout << "Client wanted to send data with payload size " << packet.payloadLength << " but we only handle a maximum of " << MAX_PACKET_PAYLOAD_SIZE << "." << std::endl;
		exit(-1); // nope. Just nope.
	}

	uint8_t* payload = new uint8_t[packet.payloadLength];
	SSL_read(_ssl, payload, packet.payloadLength);

	receivedPacket.emit(packet, payload, this);
}

SecNet::~SecNet()
{
	SSL_shutdown(_ssl);
	close(_socket);
	SSL_free(_ssl);

	exit(0);
}

void SecNet::sendPacket(Packet packet, uint8_t* payload)
{
	SSL_write(_ssl, &packet, sizeof(packet));
	SSL_write(_ssl, payload, packet.payloadLength);
}

void SecNet::handleWebSocket(Packet* packet)
{
	const int MaxHeaders = 15;
	
	char initialData[sizeof(Packet) + 1];
	bzero(initialData, sizeof(Packet) + 1);
	memcpy(initialData, packet, sizeof(Packet));

	std::string headerLines[MaxHeaders]; // we only support the first 15 headers
	headerLines[0] = "";

	int headers = 0;
	bool headerEnd = false;

	for(unsigned int i = 0; i < sizeof(Packet); i++)
	{
		char c = initialData[i];

		if(c != '\r' && c != '\n')
		{
			headerLines[headers] += c;
		}
		else if(c == '\r')
		{
			headers++;
			headerLines[headers] = "";
		}
	}

	while(!headerEnd && headers < MaxHeaders)
	{
		char c = 0;
		SSL_read(_ssl, &c, 1);

		if(c != '\r' && c != '\n')
		{
			headerLines[headers] += c;
		}
		else if(c == '\r')
		{
			if(headerLines[headers] == "")
			{
				headerEnd = true;
				headers--;
				break;
			}

			headers++;
			headerLines[headers] = "";
		}
	}

	std::string protocol, key, version;

	for(int i = 1; i < headers; i++)
	{
		std::string headerName = "";
		std::string headerContent = "";

		unsigned int j;
		for(j = 0; j < headerLines[i].size(); j++)
		{
			char c = headerLines[i].at(j);

			if(c != ':')
				headerName += c;
			else
				break;
		}

		j += 2; // skip : and the following space

		for(; j < headerLines[i].size(); j++)
		{
			char c = headerLines[i].at(j);
			headerContent += c;
		}

		if(headerName == "Sec-WebSocket-Protocol")
			protocol = headerContent;
		else if(headerName == "Sec-WebSocket-Version")
			version = headerContent;
		else if(headerName == "Sec-WebSocket-Key")
			key = headerContent;
	}

	if(version != "13")
	{
		std::cout << "Unsupported WebSocket version. Got: " << version << ", but we only support 13" << std::endl;
		exit(-1);
	}

	if(protocol != "shepoo")
	{
		std::cout << "Unsupported protocol. Got: " << protocol << ", but we only support shepoo" << std::endl;
		exit(-1);
	}

	SHA_CTX shaContext;
	unsigned char hash[SHA_DIGEST_LENGTH];

	SHA1_Init(&shaContext);
	SHA1_Update(&shaContext, key.c_str(), key.size());
	SHA1_Update(&shaContext, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
	SHA1_Final(hash, &shaContext);

	char acceptKey[30];
	bzero(acceptKey, 30);
	// hidden gem in resolv.h
	b64_ntop(hash, sizeof(hash), acceptKey, 29);

	std::string answer = std::string("HTTP/1.1 101 Switching Protocols\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Accept: ") + acceptKey + std::string("\r\n\
Sec-WebSocket-Protocol: shepoo\r\n\
\r\n");

	SSL_write(_ssl, answer.c_str(), answer.size());

	uint8_t tmp[3];
	SSL_read(_ssl, tmp, 3);

	handleWebSocketFrame();
}

void SecNet::handleWebSocketFrame()
{
	WebSocketFrameHeader header;
	bzero(&header, sizeof(header));
	SSL_read(_ssl, &header, sizeof(header));

	uint64_t payloadLength = header.payloadLen;

	if(payloadLength == 126)
	{
		uint16_t payloadLen16;
		SSL_read(_ssl, &payloadLen16, sizeof(payloadLen16));
		payloadLength = payloadLen16;
	}
	else if(payloadLength == 127)
	{
		uint64_t payloadLen64;
		SSL_read(_ssl, &payloadLen64, sizeof(payloadLen64));
		payloadLength = payloadLen64;
	}

	uint32_t maskingKey = 0;

	if(header.maskFlag)
	{
		SSL_read(_ssl, &maskingKey, sizeof(maskingKey));
	}

	uint8_t* payload = new uint8_t[payloadLength+1];
	payload[payloadLength] = 0;
	SSL_read(_ssl, payload, payloadLength);

	if(header.maskFlag)
	{
		// urghs ... it's masked ... WHY THE FUCK DO THEY DO THIS?! WE HAVE TLS!

		for(uint64_t i = 0; i < payloadLength; i++)
		{
			payload[i] = payload[i] ^ ((uint8_t*)&maskingKey)[i % 4];
		}
	}

	if(header.opcode != 0x02)
	{
		std::cout << "We only support binary packages!" << std::endl;
		exit(-1);
	}

	if(payloadLength < sizeof(Packet))
	{
		std::cout << "WebSocket Payload must contain at least one Shepoo Packet Header" << std::endl;
		exit(-1);
	}

	Packet* packet = (Packet*)payload;

	if(payloadLength < (sizeof(Packet) + packet->payloadLength))
	{
		// check if it is fragmented
		if(!header.fin)
		{
			// it is fragmented ..
			std::cout << "Fragmented WebSocket message!" << std::endl;
			exit(-1);
		}
		else
		{
			std::cout << "WebSocket frame truncated but not fragmented!" << std::endl;
			exit(-1);
		}
	}

	receivedPacket.emit(*packet, (uint8_t*)(payload + sizeof(Packet)), this);
}

void SecNet::Initialize(std::string listenAddress, std::string certificateFile, std::string privateKeyFile, std::string dhParamFile, std::string tlsCipherList)
{
	SSL_library_init();
	SSL_load_error_strings();
	SSL_METHOD* serverMethod = (SSL_METHOD*)TLSv1_2_server_method();
	SSL_CTX* serverContext = SSL_CTX_new(serverMethod);

	if(!serverContext)
	{
		std::cout << "SSL_CTX_new() failed!" << std::endl;
		exit(-1);
	}

	int status = SSL_CTX_use_certificate_file(serverContext, certificateFile.c_str(), SSL_FILETYPE_PEM);

	if(status <= 0)
	{
		std::cout << "Could not load certificate-file!" << std::endl;
		exit(-1);
	}

	status = SSL_CTX_use_PrivateKey_file(serverContext, privateKeyFile.c_str(), SSL_FILETYPE_PEM);

	if(status <= 0)
	{
		std::cout << "Could not load private-key-file!" << std::endl;
		exit(-1);
	}

	if(!SSL_CTX_check_private_key(serverContext))
	{
		std::cout << "SSL-certificate and private-key don't belong together!" << std::endl;
		exit(-1);
	}

	long sslopt = SSL_OP_SINGLE_DH_USE;
	status = SSL_CTX_set_options(serverContext, sslopt);

	if(status <= 0)
	{
		std::cout << "Could not set SSL options!" << std::endl;
		exit(-1);
	}

	FILE* dhFile = fopen(dhParamFile.c_str(), "rb");
	DH* dh = PEM_read_DHparams(dhFile, NULL, NULL, NULL);
	fclose(dhFile);
	status = SSL_CTX_set_tmp_dh(serverContext, dh);

	if(status <= 0)
	{
		std::cout << "Could not load dhparam-file!" << std::endl;
		exit(-1);
	}

	if(tlsCipherList != "")
	{
		status = SSL_CTX_set_cipher_list(serverContext, tlsCipherList.c_str());

		if(status <= 0)
		{
			std::cout << "Could not set TLS cipher list!" << std::endl;
			exit(-1);
		}
	}

	_serverSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	if(_serverSocket == -1)
	{
		std::cout << "socket() failed!" << std::endl;
		exit(-1);
	}

	struct sockaddr_in6 address;
	bzero(&address, sizeof(address));
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(SecNet::DefaultPort);
	address.sin6_addr = in6addr_any;

	status = bind(_serverSocket, (struct sockaddr*)&address, sizeof(address));

	if(status != 0)
	{
		std::cout << "bind() failed!" << std::endl;
		exit(-1);
	}

	status = listen(_serverSocket, SOMAXCONN);

	if(status != 0)
	{
		std::cout << "listen() failed!" << std::endl;
		exit(-1);
	}

	SecNet::_listening = true;

	ListenLoop(serverContext);
}

void SecNet::ListenLoop(SSL_CTX* serverContext)
{
	sockaddr_in6 clientAddress;
	socklen_t caLength = sizeof(clientAddress);

	while(SecNet::_listening)
	{
		int newSocket = accept(SecNet::_serverSocket, (struct sockaddr*)&clientAddress, &caLength);

		if(newSocket < 0)
		{
			std::cout << "Error accepting new connection: " << errno << std::endl;
		}
		else
		{
			__pid_t clientProcess = fork();

			if(clientProcess == 0)
			{
				// we are the new fork
				SecNet clientInstance(newSocket, serverContext);
				exit(-1);
			}
			else
			{
				close(newSocket); // we are the server process

				char* ip = new char[INET6_ADDRSTRLEN];
				getnameinfo((struct sockaddr*)&clientAddress, caLength, ip, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST);

				std::cout << "Forked for client-handling. Client-IP: " << ip << std::endl;
			}
		}
	}
}

int SecNet::sslErrorCallback(const char* message, size_t length, void* userData)
{
	std::cout << "SSL: " << message << std::endl;
	return 0;
}
