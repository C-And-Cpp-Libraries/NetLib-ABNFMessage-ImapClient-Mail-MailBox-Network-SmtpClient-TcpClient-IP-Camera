#if defined(_WIN32) || defined(WIN32) || defined (_WIN64) || defined (WIN64)
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#if defined(__gnu_linux__) || defined (__linux__)
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#endif

#if defined(__unix__)
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <memory.h>

#if defined(_WIN32) || defined(WIN32) || defined (_WIN64) || defined (WIN64)
#define socklen_t int
#define socketerror  ::WSAGetLastError()
#define socketioctl(socket, flag, var) ioctlsocket(socket, flag, (u_long*)&var)
#ifndef ERESTART
#define ERESTART 999
#endif
#endif

#if defined(__gnu_linux__) || defined (__linux__)
#define socketerror   errno
#define socketioctl(socket, flag, var) ioctlsocket(socket, flag, (char*)&var)
#define closesocket(n) close(n) 
#define SOCKET long
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

#if defined(__unix__)
#define socketerror   errno
#define socketioctl(socket, flag, var) ioctl(socket, flag, (char*)&var)
#define closesocket(n) close(n) 
#define SOCKET long
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#ifndef ERESTART
#define ERESTART 999
#endif
#endif

#include "TcpClient.hpp"
#include "Network.hpp"

namespace CoreLibrary
{
	class SocketReference
	{
	public:
		bool				_Connected;
		unsigned long		_Socket;
		sockaddr_in			_ServerAddress;
		GenericString		_ServerName;
		int					_ServerPort;
		int					_PreFetchedBufferSize;
		unsigned char*		_PreFetchedBuffer;
		int					_PacketSize;
		unsigned char*		_Packet;
		char				_PacketDelimeter[32];
		PacketBehaviour		_Phv;
	};

	TcpClient::TcpClient()
	{
		_SocketReference = new SocketReference();
		_SocketReference->_Socket = 0;
		_SocketReference->_Connected = false;
		memset((void*)&_SocketReference->_ServerAddress, 0, sizeof(sockaddr_in));
		_SocketReference->_PreFetchedBufferSize = 0;
		_SocketReference->_PreFetchedBuffer = nullptr;
		_SocketReference->_PacketSize = 0;
		_SocketReference->_Packet = nullptr;
		_SocketReference->_Phv = Undefined;
	}

	TcpClient::TcpClient(int inSocket, bool requireSSL)
	{
		_SocketReference = new SocketReference();
		_SocketReference->_Socket = inSocket;
		_SocketReference->_Connected = true;
		memset((void*)&_SocketReference->_ServerAddress, 0, sizeof(sockaddr_in));
		_SocketReference->_PreFetchedBufferSize = 0;
		_SocketReference->_PreFetchedBuffer = nullptr;
		_SocketReference->_PacketSize = 0;
		_SocketReference->_Packet = nullptr;
		_SocketReference->_Phv = Undefined;
	}

	TcpClient::TcpClient(const TcpClient& other)
	{
		_SocketReference = new SocketReference();
		memset((void*)&_SocketReference->_ServerAddress, 0, sizeof(sockaddr_in));

		_SocketReference->_PreFetchedBufferSize = 0;

		if (_SocketReference->_PreFetchedBuffer != nullptr)
		{
			delete _SocketReference->_PreFetchedBuffer;
			_SocketReference->_PreFetchedBuffer = nullptr;
		}

		_SocketReference->_PacketSize = 0;

		if (_SocketReference->_Packet != nullptr)
		{
			delete _SocketReference->_Packet;
			_SocketReference->_Packet = nullptr;
		}

		_SocketReference->_Socket = other._SocketReference->_Socket;
		_SocketReference->_Connected = other._SocketReference->_Connected;
		memcpy((void*)&_SocketReference->_ServerAddress, (void*)&other._SocketReference->_ServerAddress, sizeof(sockaddr_in));

		if (other._SocketReference->_PreFetchedBufferSize > 0)
		{
			_SocketReference->_PreFetchedBuffer = new unsigned char[other._SocketReference->_PreFetchedBufferSize];
			_SocketReference->_PreFetchedBufferSize = other._SocketReference->_PreFetchedBufferSize;
			memcpy((unsigned char*)&_SocketReference->_PreFetchedBuffer, (unsigned char*)&other._SocketReference->_PreFetchedBuffer, _SocketReference->_PreFetchedBufferSize);
		}

		if (other._SocketReference->_PacketSize > 0)
		{
			_SocketReference->_Packet = new unsigned char[other._SocketReference->_PacketSize];
			_SocketReference->_PacketSize = other._SocketReference->_PacketSize;
			memcpy((unsigned char*)&_SocketReference->_Packet, (unsigned char*)&other._SocketReference->_Packet, _SocketReference->_PacketSize);
		}

		_SocketReference->_Phv = other._SocketReference->_Phv;
	}

	TcpClient& TcpClient::operator=(const TcpClient& other)
	{
		memset((void*)&_SocketReference->_ServerAddress, 0, sizeof(sockaddr_in));

		_SocketReference->_PreFetchedBufferSize = 0;

		if (_SocketReference->_PreFetchedBuffer != nullptr)
		{
			delete _SocketReference->_PreFetchedBuffer;
			_SocketReference->_PreFetchedBuffer = nullptr;
		}

		_SocketReference->_PacketSize = 0;

		if (_SocketReference->_Packet != nullptr)
		{
			delete _SocketReference->_Packet;
			_SocketReference->_Packet = nullptr;
		}

		_SocketReference->_Socket = other._SocketReference->_Socket;
		_SocketReference->_Connected = other._SocketReference->_Connected;
		memcpy((void*)&_SocketReference->_ServerAddress, (void*)&other._SocketReference->_ServerAddress, sizeof(sockaddr_in));

		if (other._SocketReference->_PreFetchedBufferSize > 0)
		{
			_SocketReference->_PreFetchedBuffer = new unsigned char[other._SocketReference->_PreFetchedBufferSize];
			_SocketReference->_PreFetchedBufferSize = other._SocketReference->_PreFetchedBufferSize;
			memcpy((unsigned char*)&_SocketReference->_PreFetchedBuffer, (unsigned char*)&other._SocketReference->_PreFetchedBuffer, _SocketReference->_PreFetchedBufferSize);
		}

		if (other._SocketReference->_PacketSize > 0)
		{
			_SocketReference->_Packet = new unsigned char[other._SocketReference->_PacketSize];
			_SocketReference->_PacketSize = other._SocketReference->_PacketSize;
			memcpy((unsigned char*)&_SocketReference->_Packet, (unsigned char*)&other._SocketReference->_Packet, _SocketReference->_PacketSize);
		}

		_SocketReference->_Phv = other._SocketReference->_Phv;
		return *this;
	}

	TcpClient::~TcpClient()
	{
		closeSocket();
		while (isConnected() == true)
		{
		}

		if (_SocketReference != nullptr)
		{
			if (_SocketReference->_PreFetchedBuffer != nullptr)
			{
				delete _SocketReference->_PreFetchedBuffer;
			}

			delete _SocketReference;
		}
	}

	void TcpClient::setPacketDelimeter(char * str)
	{
		_SocketReference->_Phv = Delimited;
		memcpy(_SocketReference->_PacketDelimeter, str, strlen(str));
	}

	void TcpClient::setPacketLength(long len)
	{
		_SocketReference->_Phv = FixedLength;
		_SocketReference->_PacketSize = len;
	}

	bool TcpClient::createSocket(const char* servername, int serverport, bool requireSSL)
	{
		_SocketReference->_ServerName = servername;
		_SocketReference->_ServerPort = serverport;

		_SocketReference->_ServerAddress.sin_family = AF_INET;
		_SocketReference->_ServerAddress.sin_port = htons(serverport);
		u_long nRemoteAddr;

		char ipbuffer[32] = { 0 };
		memcpy(ipbuffer, servername, 31);

		bool ip = Network::isIP4Address(ipbuffer);

		if (!ip)
		{
			hostent* pHE = gethostbyname(_SocketReference->_ServerName.buffer());
			if (pHE == 0)
			{
				nRemoteAddr = INADDR_NONE;
				return false;
			}
			nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
			_SocketReference->_ServerAddress.sin_addr.s_addr = nRemoteAddr;
		}
		else
		{
			inet_pton(AF_INET, _SocketReference->_ServerName.buffer(), &_SocketReference->_ServerAddress.sin_addr);
		}

        _SocketReference->_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (_SocketReference->_Socket == INVALID_SOCKET)
        {
            return false;
        }

		return true;
	}

	bool TcpClient::createSocket(unsigned long inSocket, bool requireSSL)
	{
		_SocketReference->_Socket = inSocket;
		_SocketReference->_Connected = true;
		return true;
	}

	bool TcpClient::connectSocket(int &returncode)
	{
		if (_SocketReference->_Connected == true)
		{
			return true;
		}

		returncode = connect(_SocketReference->_Socket, (sockaddr*)&_SocketReference->_ServerAddress, sizeof(sockaddr_in));

		if (returncode == SOCKET_ERROR)
		{
			returncode = socketerror;

			shutdown(_SocketReference->_Socket, 2);
			closesocket(_SocketReference->_Socket);
			_SocketReference->_Connected = false;

			return false;
		}

		_SocketReference->_Connected = true;
		return true;
	}

	bool TcpClient::closeSocket()
	{
		if (_SocketReference != nullptr)
		{
			if (_SocketReference->_Connected)
			{
                shutdown(_SocketReference->_Socket, 0);
                closesocket(_SocketReference->_Socket);
			}
			_SocketReference->_Connected = false;
		}
		return false;
	}

	bool TcpClient::receiveString(GenericString &ioStr, const char *delimeter)
	{
		char	buffer[1024];
		long	returnvalue;
		GenericString	data;
		GenericString  currentLine, nextLine;

		data.clear();

		if (_SocketReference->_PreFetchedBufferSize > 0)
		{
			if (strstr((char*)_SocketReference->_PreFetchedBuffer, delimeter) != 0)
			{
				GenericString temp = (char*)_SocketReference->_PreFetchedBuffer;
				temp.getKeyValuePair(currentLine, nextLine, delimeter);

				ioStr = currentLine;
				currentLine.clear();

				delete _SocketReference->_PreFetchedBuffer;
				_SocketReference->_PreFetchedBuffer = nullptr;
				_SocketReference->_PreFetchedBufferSize = nextLine.length();

				if (_SocketReference->_PreFetchedBufferSize > 0)
				{
					_SocketReference->_PreFetchedBuffer = new unsigned char[_SocketReference->_PreFetchedBufferSize + 1];
					memset(_SocketReference->_PreFetchedBuffer, 0, _SocketReference->_PreFetchedBufferSize + 1);
					memcpy(_SocketReference->_PreFetchedBuffer, nextLine.buffer(), _SocketReference->_PreFetchedBufferSize);
				}

				return true;
			}

			data = (char*)_SocketReference->_PreFetchedBuffer;
			_SocketReference->_PreFetchedBufferSize = 0;
			delete _SocketReference->_PreFetchedBuffer;
			_SocketReference->_PreFetchedBuffer = nullptr;
		}

		while (true)
		{
			memset(&buffer[0], 0, 1024);

            returnvalue = recv(_SocketReference->_Socket, &buffer[0], 1024, 0);

			if (returnvalue < 1)
			{
				int error = socketerror;
				ioStr.clear();
				_SocketReference->_Connected = false;
				return false;
			}

			data += buffer;

			if (strstr(data.buffer(), delimeter) != 0)
			{
				data.getKeyValuePair(currentLine, nextLine, delimeter);

				_SocketReference->_PreFetchedBufferSize = nextLine.length();

				if (_SocketReference->_PreFetchedBufferSize > 0)
				{
					_SocketReference->_PreFetchedBuffer = new unsigned char[_SocketReference->_PreFetchedBufferSize + 1];
					memset(_SocketReference->_PreFetchedBuffer, 0, _SocketReference->_PreFetchedBufferSize + 1);
					memcpy(_SocketReference->_PreFetchedBuffer, nextLine.buffer(), _SocketReference->_PreFetchedBufferSize);
				}

				ioStr = currentLine;

				data.clear();
				currentLine.clear();
				return true;
			}
		}
		return true;
	}

	bool TcpClient::receiveBuffer(int len)
	{
		char*	buffer = 0;
		long	bufferpos = 0;
		long	bytesread = 0;
		long	bytesleft = len;

		// If there are pre-fetched bytes left, we have to copy that first and relase memory

		if (_SocketReference->_PreFetchedBufferSize > 0)
		{
			if (_SocketReference->_Packet != nullptr)
			{
				delete _SocketReference->_Packet;
				_SocketReference->_Packet = nullptr;
			}
			_SocketReference->_Packet = new unsigned char[_SocketReference->_PreFetchedBufferSize];
			memcpy(_SocketReference->_Packet, _SocketReference->_PreFetchedBuffer, _SocketReference->_PreFetchedBufferSize);
			bytesleft = len - _SocketReference->_PreFetchedBufferSize;
			bufferpos = _SocketReference->_PreFetchedBufferSize;
			_SocketReference->_PreFetchedBufferSize = 0;
			delete _SocketReference->_PreFetchedBuffer;
			_SocketReference->_PreFetchedBuffer = nullptr;

			if (bytesleft < 1)
			{
				return true;
			}
		}

		while (true)
		{
			buffer = new char[bytesleft + 1];
			memset(buffer, 0, bytesleft + 1);

            bytesread = recv(_SocketReference->_Socket, buffer, bytesleft, 0);

			if (bytesread < 1)
			{
				int error = socketerror;
				delete[] buffer;
				_SocketReference->_Packet = nullptr;
				len = 0;
				_SocketReference->_Connected = false;
				return false;
			}

			memcpy(_SocketReference->_Packet + bufferpos, buffer, bytesread);
			delete[] buffer;

			bufferpos = bufferpos + bytesread;

			bytesleft = bytesleft - bytesread;

			if (bufferpos >= len)
			{
				return true;
			}
		}
	}

	int TcpClient::pendingPreFetchedBufferSize()
	{
		return _SocketReference->_PreFetchedBufferSize;
	}


	bool TcpClient::sendBuffer(const char* data, int &len)
	{
		if (!_SocketReference->_Connected)
		{
			return false;
		}

		long sentsize = 0;

        sentsize = send(_SocketReference->_Socket, data, len, 0);

		if (sentsize == SOCKET_ERROR)
		{
			return false;
		}

		return true;
	}

	bool TcpClient::sendString(const GenericString &str)
	{
		int len = str.length();
		bool ret = sendBuffer(str.buffer(), len);
		return ret;
	}

	bool TcpClient::isConnected()
	{
		if (_SocketReference == nullptr)
			return false;
		return _SocketReference->_Connected;
	}

	unsigned long TcpClient::getSocket()
	{
		return _SocketReference->_Socket;
	}

	void TcpClient::dataReceived(const char *ptr, int len)
	{

	}

	DescriptorType TcpClient::descriptorType()
	{
		return FDClient;
	}

	GenericString* TcpClient::certificateName()
	{
        return nullptr;
	}
}
