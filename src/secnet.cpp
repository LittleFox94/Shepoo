#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <openssl/err.h>

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

	if(*(int*)&packet == 'GET ' /*0x20544547*/) // don't hurt me .. it's just "GET " as integer
	{
		// meh .. It's HTTP, probably Websocket as we use this one ...
		std::cout << "Oh noes ... it's HTTP ..." << std::endl;
	}

	if(packet.payloadLength > MAX_PACKET_PAYLOAD_SIZE)
		exit(-1); // nope. Just nope.

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
