/*
 * Mail-Sending Program
 * MailSenderSmtp.hh
 *
 *  Created on: Oct 18, 2010
 *      Author: Joseph Lee
 *	
 *		CS 300: Assignment #2
 *	Instructor: Bart Massey
 */

/*	Copyright (c) 2010 Joseph Lee

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files
	(the "Software"), to deal in the Software without restriction,
	including without limitation the rights	to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software,
	and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER	LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

	*/

#ifndef MAILSENDERSMTP_HH_
#define MAILSENDERSMTP_HH_

#include "MailSender.hh"
#include <iostream>
#include <string>

using namespace std;

/*
 * MailSenderSmtp object
 * Derived from MailSender (abstract base class)
 * Send contents of an RFC-821 formatted e-mail through
 * an SMTP server relay, interfacing w/ the server using
 * the RFC-822 Server-Client model.
 * Standard SMTP protocol, port 25 w/ no authorization
 * protocol.
 */
class MailSenderSmtp : public MailSender
{
  public:
			 MailSenderSmtp(const string &filename): MailSender(filename) { }
			~MailSenderSmtp() { }

	// Send email to relay host via TCP/IPv4 and interfacing

	// with an SMTP server.

	int			send(const string &host_to,
					 const string &envelope_from,
					 const string &envelope_to);

  private:

	enum Port { Smtp = 25 };	// Port #: 25 (SMTP)

	 // Create socket, connect to host.

	int			open_clientfd(const string &host);

	 // Interface w/ SMTP server to send contents of email file

	 // using write/recv methods via sockets.

	int			smtp_client(int clientfd,
							const string &envelope_from,
							const string &envelope_to);

	 // Format write/read commands, call those functions.

	 // Compare server responce to expected response (param 4).

	 // If server response is unexpected or lost packets exist,

	 // attempt transfer a second time.

	int			send_recv_cmd(int sockfd,
							  const string &cmd,
							  const string &param,
							  const string &confirm = "250");
							  // Default server reply: 'OK'

};

#endif /* MAILSENDERSMTP_HH_ */
