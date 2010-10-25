/*
 * Mail-Sending Program
 * MailSenderSmtp.cc
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

#include "MailSenderSmtp.hh"
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

using namespace std;

const int MAX_BUF = 1024;	// Size of receive buffer (1 kb)

/*
 * MailSender public send method
 * Emails contents of instantiated filename email file to specified
 * relay host (host_to parameter).
 * TCP/IPv4 socket is created and used to interface w/ relay host
 * using SMTP client-server protocol.
 * Uses socket function "write(...)" instead of "send(...)" because
 * of name clash w/ this function.
 * 	- open_clientfd: creates socket/file descriptor/connects to host
 * 	- smtp_client: interface w/ host using SMTP commands
 * @args:	relay host domain (const string &host_to)
 * 			email sender (const string &envelope_from)
 * 			email recipient (const string &envelope_to)
 * @return:	0 (success)
 *  -error: -1 (errno specified on connection errors in clientfd(...))
 *  		-1 (SMTP error, server response code)
 */
int
MailSenderSmtp::send(const string &host_to,
					 const string &envelope_from,
					 const string &envelope_to)
{

	int				clientfd;		// Socket file descriptor

	// Set client file descrip, make connection to host
	if ((clientfd = open_clientfd(host_to)) == -1) {

		// Error creating socket/making connection
		// Check "errno"
		return -1;

	}

	// Interface with SMTP server.
	if (smtp_client(clientfd, envelope_from, envelope_to) != 0) {

		// Error interfacting w/server.
		// Check recv'd SMTP message.
		return -1;

	}

	close(clientfd);	// Close socket.

	return 0;

}

/*
 * Create TCP/IPv4 Socket to specified host domain, SMTP port (25)
 * @args:	 SMTP server hostname
 * @return:	 file descriptor <int> (on success)
 * - error:  -1, errno flag
 */
int
MailSenderSmtp::open_clientfd(const string &host)
{

	int				clientfd;	// File descriptor.
	hostent			*hp;
	sockaddr_in		serveraddr;

	// Create socket file descriptor: TCP/IPv4
	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {

		return -1;	// check errno for cause of error

	}

	if ((hp = gethostbyname(host.c_str())) == NULL) {

		return -1;	// check errno for cause of error

	}

	// Init sockaddr_in to '0'
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;	// IPv4
	bcopy((char *)hp->h_addr,
		  (char *)&serveraddr.sin_addr.s_addr,
		  hp->h_length);
	serveraddr.sin_port = htons(Smtp);	// Convert to network-byte order

	if(connect(clientfd,
			   (sockaddr *)&serveraddr,
			   sizeof(serveraddr)) < 0) {

		return -1; // Error, check errno for connection error

	}

	return clientfd;	// Valid file descriptor

}

/*
 * Interface w/ SMTP server to send contents of email file
 * using read/write(...) methods via sockets. No longer uses
 * "send/recv(...)" because of name clashing of MailSender.send(...)
 * with the socket function "send(...)".
 * @args:	file descriptor, sender e-mail address, recipient
 * 			e-mail address.
 * @return:	0 (on success)
 * - error: -1 (SMTP server response error)
 *
 * SMTP Server/Client Dialogue:
 * 	Server: Acknowledge client connection
 * 	"HELO <client-domain>"	(Server OK: "250...")
 * 	"MAIL FROM: <sender>"	(Server OK: "250...")
 * 	"RCPT TO: <recipient>"	(Server OK: "250...")
 * 	"DATA"					(Server OK: "354...")
 * 	Input file stream: open filename
 * 	WHILE getchar != EOF
 * 		THEN add file character to buffer
 * 	SEND buffer
 * 	"QUIT"
 * 	close file stream
 */
int
MailSenderSmtp::smtp_client(int clientfd,
							const string &envelope_from,
							const string &envelope_to)
{

	string			to_send;			// Send buffer
	char			buffer[MAX_BUF];	// Recv buffer, 1024 bytes
	int				sent_bytes,			// # bytes received
					recv_bytes;			// and sent.

	ifstream fin(get_filename().c_str());	// Open email file

	// Server confirm connection
	recv_bytes = read(clientfd, buffer, MAX_BUF - 1);
	buffer[recv_bytes] = '\0';
	cout << buffer << endl;

	// Check for error in greeting.
	if (strncmp(buffer, "220", 3)) {

		return -1; 		// Error establishing connection

	}

	// HELO command
	if (send_recv_cmd(clientfd,
					  "HELO ",
					  envelope_from.substr(		// Domain of sender
							  envelope_from.find('@', 0) + 1)) != 0) {

		return -1;	// Error

	}

	// MAIL FROM command
	if (send_recv_cmd(clientfd,
					  "MAIL FROM:",
					  envelope_from) != 0) {

		return -1;	// Error

	}

	// RCPT TO command
	if (send_recv_cmd(clientfd,
					  "RCPT TO:",
					  envelope_to) != 0) {

		return -1;	// Error

	}

	// DATA command
	if (send_recv_cmd(clientfd,
					  "DATA",
					  "",
					  "354") != 0) {

		return -1;	// Error

	}

	to_send.clear();	// Clear send buffer

	// Read File data
	while (fin.getline(buffer, MAX_BUF)) {

		to_send.append(buffer);
		to_send.append("\n");

	}

	fin.close();	// Close file stream.

	cout << "<Start \"" << get_filename() << "\">\n\n" << to_send
		 << "\n\n<End of \"" << get_filename() << "\">\n\n";
	to_send.append("\r\n.\r\n");	// End of data: <CRLF>.<CRLF>

	sent_bytes = write(clientfd, to_send.c_str(), to_send.length());

	// Server confirm email contents, attempts to relay e-mail
	recv_bytes = read(clientfd, buffer, MAX_BUF - 1);
	buffer[recv_bytes] = '\0';
	cout << "S: " << buffer;

	// Check if email is blocked by SpamAssassin
	if (strncmp("550", buffer, 3) == 0) {

		sent_bytes = write(clientfd, "QUIT", 4);
		return -1;

	}

	// Client: QUIT command
	sent_bytes = write(clientfd, "QUIT", 4);

	// Server closes connection

	return 0;		// Return success.

}

/*
 * SMTP client-server command handler
 * Construct command to send to server, compare server
 * response to one that is expected.
 * Allow for two attempts in the case of lost packets
 * in communication and/or unexpected server response.
 * Uses methods "read(...)" and "write(...)" via sockets.
 * @args:	socket file descrip (int sockfd)
 * 			command to issue (const string &cmd)
 * 			command parameter (const string &param)
 * 			expected server reply (const string &confirm)
 * 				(by default: "250")
 * @return:	0  (success)
 * - error: -1 (failure, lost packets/unexpected reply)
 */
int
MailSenderSmtp::send_recv_cmd(int sockfd,
							  const string &cmd,
							  const string &param,
							  const string &confirm)
{

	string			to_send;			// Send buffer
	char			buf[MAX_BUF];		// Recv buffer
	int				sent_bytes,			// Size of sent command
					recv_bytes;			// Size of recv command

	for(int i = 0; i < 2; i++) {	// Allow 2 attempts

		to_send = cmd;

		// Surround param w/ angle brackets on attempt 2
		// Some servers have required angle bracets around
		// email address parameters.
		if (i == 1 &&
			param.length() > 0)	{

			to_send.append("<");

		}

		to_send.append(param);

		if (i == 1 &&
			param.length() > 0) {

			to_send.append(">");

		}

		to_send.append("\n");
		cout << "C: " << to_send;

		// Send command
		if ((sent_bytes = write(sockfd, to_send.c_str(),
			 to_send.length())) < (int)to_send.length()) {

			if (i > 0) {		// 2nd attempt

				return -1;		// Lost bytes or empty command

			}

		}

		// Receive server reply
		if ((recv_bytes = read(sockfd, buf, MAX_BUF - 1)) < 1) {

			if (i > 0) {		// 2nd attempt

				return -1;		// Lost bytes from server

			}

		}

		buf[recv_bytes] = '\0';
		cout << "S: " << buf << endl;

		// Server confirmation or repeated command
		if (strncmp(buf, confirm.c_str(), 3) == 0 ||
			strncmp(buf, "503", 3) == 0) {	// 503: Repeated cmd

			return 0;	// Confirmed, success

		}

	}

	return -1;		// Fail

}







