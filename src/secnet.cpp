#include "../include/secnet.h"

void SecNet::Initialize(std::string listen, std::string certificateFile, std::string privateKeyFile, std::string dhParamFile, std::string tlsCipherList)
{
	SSL_library_init();
	SSL_load_error_strings();
	SSL_METHOD* serverMethod = (SSL_METHOD*)TLSv1_2_server_method();
	SSL_CTX* serverContext = SSL_CTX_new(serverMethod);

	if(!sslServerContext)
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
		std::cout << "Could not load private-key-file!" << std::endl:
		exit(-1);
	}

	if(!SSL_CTX_check_private_key(serverContext))
	{
		std::cout << "SSL-certificate and private-key don't belong together!" << std::endl;
		exit(-1);
	}

	long sslopt = SSL_OP_SINGLE_DH_USE | SSL_OP_NO_TICKET | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
	status = SSL_CTX_set_options(serverContext, sslopt);

	if(status <= 0)
	{
		std::cout << "Could not set SSL options!" << std::endl;
		exit(-1);
	}

	DH* dh = PEM_read_DHparams(dhParamFile.c_str(), NULL, NULL, NULL);
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
	address.sin6_port = htons(DefaultPort);
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

	ListenLoop(serverContext);
}
