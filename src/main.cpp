#include <iostream>
#include <getopt.h>

#include "../config.h"
#include "../include/secnet.h"

void help();

int main(int argc, char* argv[])
{
	std::string blockStorageFile;
	std::string shuffleDBFile;

	std::string logFile;

	std::string tlsCertificateFile;
	std::string tlsPrivateKeyFile;
	std::string tlsDHParamFile;
	std::string tlsCipherSuite;

	std::string listenAddress;

	int daemonize = 0;
	int yes_i_am_sure = 0;

	while(1)
	{
		static struct option longOptions[] =
		{
			{"daemonize", no_argument, &daemonize, 1},
			{"yes-i-am-sure", no_argument, &yes_i_am_sure, 1},

			{"blockStorageFile", required_argument, 0, 'a'},
			{"shuffleDBFile", required_argument, 0, 'b'},
			
			{"logFile", required_argument, 0, 'c'},

			{"tlsCertificateFile", required_argument, 0, 'd'},
			{"tlsPrivateKeyFile", required_argument, 0, 'e'},
			{"tlsDHParamFile", required_argument, 0, 'f'},
			{"tlsCipherSuite", required_argument, 0, 'g'},

			{"listenAddress", required_argument, 0, 'i'},

			{"help", no_argument, 0, 'h'},
			{"version", no_argument, 0, 'V'},
	
			{0, 0, 0, 0}
		};

		int optionIndex = 0;
		int c = getopt_long_only(argc, argv, "Vh", longOptions, &optionIndex);

		if(c == -1)
			break;

		switch(c)
		{
			case 0:
				if(longOptions[optionIndex].flag != 0)
					break;
	
				// ehm ... dunno ...
				break;
			case 'a':
				blockStorageFile = std::string(optarg);
				break;
			case 'b':
				shuffleDBFile = std::string(optarg);
				break;
			case 'c':
				logFile = std::string(optarg);
				break;
			case 'd':
				tlsCertificateFile = std::string(optarg);
				break;
			case 'e':
				tlsPrivateKeyFile = std::string(optarg);
				break;
			case 'f':
				tlsDHParamFile = std::string(optarg);
				break;
			case 'g':
				tlsCipherSuite = std::string(optarg);
				break;
			case 'i':
				listenAddress = std::string(optarg);
				break;
			case 'h':
				help();
				return 0;
				break;
			case 'V':
				std::cout << "Shepoo Version " << VERSION << std::endl;
				std::cout << "This program is distributed under the terms of the MIT license" << std::endl;
				return 0;
				break;
		}
	}

	bool missingArguments = false;

	if(blockStorageFile == "" && shuffleDBFile == "")
		missingArguments = true;

	if(blockStorageFile != "" && shuffleDBFile != "" && yes_i_am_sure == 0)
		missingArguments = true;

	if(tlsCertificateFile == "" || tlsPrivateKeyFile == "" || tlsDHParamFile == "")
		missingArguments = true;

	if(missingArguments)
	{
		help();
		return 0;
	}
}

void help()
{
	std::cout << "Usage:" << std::endl
		<< "shepoo [arguments]" << std::endl << std::endl
		<< std::endl
		<< "--blockStorageFile <filePath>    Path to the file where the blocks are stored" << std::endl
		<< "--shuffleDBFile <filePath>       Path to the database-file of the shuffle-server" << std::endl
		<< "--yes-i-am-sure                  If you want one server to be blockstorage- and" << std::endl
		<< "                                   shuffle-server, you have to give this argument" << std::endl
		<< "--tlsCertificateFile <filePath>  Path to the TLS certificate" << std::endl
		<< "--tlsPrivateKeyFile <filePath>   Path to the private key for the given certificate" << std::endl
		<< "--tlsDHParamFile <filePath>      Path to the dhparam file" << std::endl
		<< "--tlsCipherSuite <ciphersuite>   OpenSSL ciphersuite string" << std::endl
		<< "--listenAddress <listenaddress>  The address and port we should listen on" << std::endl
		<< "--logfile <filepath>             Where to put log-messages in daemon mode" << std::endl
		<< "--daemonize                      The program will daemonize itself on startup" << std::endl
		<< std::endl
		<< "There is a better documentation online." << std::endl << std::endl;
}
