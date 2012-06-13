/*
 * tcpserver.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ibrcommon/config.h"
#include "ibrcommon/net/tcpserver.h"
#include "ibrcommon/thread/MutexLock.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

namespace ibrcommon
{
	tcpserver::tcpserver()
	 : _socket()
	{
	}

	tcpserver::tcpserver(const ibrcommon::File &s)
	 : _socket()
	{
		_socket.bind(s);

		// listen on the socket, max. 5 concurrent awaiting connections
		listen(5);
	}

	void tcpserver::listen(int connections)
	{
		_socket.listen(connections);
	}

	void tcpserver::bind(int port, bool reuseaddr)
	{
		// enable reuse address if requested
		if (reuseaddr)
		{
			_socket.set(vsocket::VSOCKET_REUSEADDR);
		}

		// bind to the socket
		_socket.bind(port);
		_socket.listen(5);

		// set linger socket option
		_socket.set(vsocket::VSOCKET_LINGER);

		// set socket to nonblocking
		_socket.set(vsocket::VSOCKET_NONBLOCKING);
	}

	void tcpserver::bind(const vinterface &net, int port, bool reuseaddr)
	{
		// enable reuse address if requested
		if (reuseaddr)
		{
			_socket.set(vsocket::VSOCKET_REUSEADDR);
		}

		// bind to the socket
		_socket.bind(net, port);


		_socket.listen(5);

		// set linger socket option
		_socket.set(vsocket::VSOCKET_LINGER);

		// set socket to nonblocking
		_socket.set(vsocket::VSOCKET_NONBLOCKING);
	}

	tcpserver::~tcpserver()
	{
		close();
	}

	void tcpserver::close()
	{
		_socket.close();
	}

	void tcpserver::shutdown()
	{
		_socket.shutdown();
	}

	tcpstream* tcpserver::accept()
	{
		std::list<int> fds;

		while (true)
		{
			_socket.select(fds, NULL);

			for (std::list<int>::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
			{
				int fd = *iter;

				struct sockaddr_in cliaddr;
				socklen_t len;
				len = sizeof(cliaddr);

				int new_fd = ::accept(fd, (struct sockaddr *) &cliaddr, &len );

				if (new_fd <= 0)
				{
					throw vsocket_exception("accept failed");
				}

				try {
					return new tcpstream(new_fd);
				} catch (const ibrcommon::Exception&) {
					// creation failed
					::close(new_fd);
				}
			}
		}

		throw vsocket_exception("tcpserver down");
	}
}
