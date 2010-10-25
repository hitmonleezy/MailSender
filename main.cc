/*
 * Mail-Sending Program
 * main.cpp
 *
 *  Created on: Oct 8, 2010
 *      Author: Joseph Lee
 *
 *      CS 300: Assignment #2
 *  Instructor: Bart Massey
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

/*
 * This program emails a file formatted according to RFC-821, supplied
 * by file name as a command-line argument. The email's sender and
 * recipient email addresses are obtained from the file and sent using
 * the MailSender object. Its derived class MailSenderSmtp sends the
 * contents of the file via SMTP interface through a specified relay
 * server (by default, host "mailhost.cecs.pdx.edu" port 25).
 */

#include "MailSenderSmtp.hh"
#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <sstream>
#include <cmath>
#include <cstring>
#include <cerrno>

using namespace std;

const string	ConfigFile = "mailsender.conf";		// Config filename

// Driver function, receives command-line file name,

// process email file information/address, instantiate

// MailSender object to send email.

int				Driver(const string &filename);

// Find sender/recipient email address.

int				GetEnvelope(const string &filename,
							string &env_from,
							string &env_to);

// Verify validity of the format/syntax of an email address.

int				CheckEmailSyntax(const string &addr);

// Load relay host from configuration file

int				LoadHost(string &host, int &port, string &auth);

// Parse configuration file data (string).

int				ParseConfig(string &parsed,
							string &buffer,
							string tag,
							int pos = 0);

// Parse configuration file data (int).

int				ParseConfig(int &parsed,
							string &buffer,
							string tag,
							int pos = 0);

int
main(int argc, char **argv) {	// Single cmd-line arg expected.

	string			filename;	// Cmd-line arg: email filename.

	// Confirm command-line arguments
	if (argc != 2) {

		cout << "Error, invalid arguments: " << argc << endl;
		return 1;	// Error, exit program.

	}

	filename = argv[1];

	if (Driver(filename) <= 0) {	// Driver function.

		return 1;	// Error.

	}

	return 0;		// Success, end program.

}

/*
 * Driver method
 * Use GetEnvelope to retrieve email address information.
 * Instantiate MailSenderSmtp object with email file
 * MailSenderSmtp.Send to send contents of email to specified
 * addresses to SMTP server: host.
 * @args: filename string
 * @return: 0 (on success)
 * -errors: -1 (File not found, improper email address syntax,
 * 				connection error, SMTP connection error)
 * 				Handled by errno or SMTP server replies.
 *
 */
int
Driver(const string &filename)
{

	string			env_from,	// Email sender address
					env_to,		// Email recipient address
					hostname,	// SMTP relay server hostname
					auth;		// Hostname authorization type
	int				port;		// Hostname port number
	MailSender		*Client;	// Ptr to object to send email

	// Extract sender & rcpt email addresses from file (header)
	if ((GetEnvelope(filename, env_from, env_to)) != 0) {

		if (errno)

			perror("File load error");

		return -1;

	}

	// Load hostname/port/authorization type
	if (LoadHost(hostname, port, auth) == -1) {

		if (errno)

			perror("Error loading configuration file");

		else

			// Errno not set, load host error.
			cout << "Configuration file load error. ";

		cout << "Please run \"config\"\n";
		return -1;

	}

	if (port == 25 && auth == "0")

		// Standard SMTP, no authorization protocol
		Client = new MailSenderSmtp(filename);

	cout << "Attempting to connect to " << hostname << endl;

	// Attempt to send e-mail.
	if (Client->send(hostname, env_from, env_to) != 0) {

		if (errno)

			perror("Connection error");		// Error: connection.


		else

			// Error: errno not set, server response error.
			cout << "SMTP Protocol Error, email not sent.\n";

	}

	return 0;	// Successfully sent email

}

/*
 * GetEnvelope method
 * Open e-mail file, process header contents and extract e-mail
 * addresses of sender & recipient.
 *
 * Search unprocessed header text for sender, recipient
 * e-mail address:
 *
 * Set input file stream: filename (of email)
 *
 * Place header into temporary buffer:
 * 	WHILE getline != end of file AND next char != '\n' (ifstream.peek)
 * 		(Header & Body separated by blank line)
 * 		THEN add line to Temp Header Buffer
 *
 * Find email addresses of sender/recipient:
 * Search Header buffer string for ':' position
 * 	WHILE Header contains next ':' (position = string.find(':', position))
 * 		IF substring Header (position - 4 to position) == "From"
 * 			IF whitespace THEN flush whitespace
 * 			IF Header[position] == '<'
 * 				THEN Sender = Header (position + 1 TO next '>')
 * 			ELSE Sender = Header (position + 1 TO whitespace)
 *
 * 		ELSE IF substring Header (position - 2 TO position) == "To"
 * 			IF whitespace THEN flush whitespace
 * 			IF Header[position] == '<'
 * 				THEN Recipient = Header (position + 1 TO next '>')
 * 			ELSE Recipient = Header (position - 2 TO whitespace)
 *
 * @args:	filename (const string &)
 * 			email address of sender (const string &env_from)
 * 			email address of recipient (const string &env_to)
 * @return: 0 (on Success)
 * -errors: -1 (File not found),
 * 			 1 (email address not found/improperly formatted)
 */
int
GetEnvelope(const string &filename,
			string &env_from,
			string &env_to)
{

	string			header;		// Unsorted e-mail header
	unsigned int	from_pos,	// Beginning of sender address string.
					to_pos,		// Beginning of recipient string.
					pos = 0;
	char			ch;
	ifstream		fin;

	fin.open(filename.c_str());
	if (fin == NULL)	// File not found.

		return -1;		// Errno set

	// Dig through email file for envelope information (header).
	// ** Header ends with 2 newline characters.
	while (fin.get(ch) && (fin.peek() != '\n' || ch != '\n')) {

		header.append(1, ch);

	}

	fin.close();

	// Find sender/recipient email address by searching "From:" & "To:"
	while ((pos = header.find(':', pos)) > 0 &&
		   pos < header.length()) {	// Search for ':'

		if (pos > 3) {	// If position of ':' can be preceded by 'From'

			// Check for Sender match
			if (strncasecmp((header.substr(pos - 4, 4)).c_str(),
							 "From", 4) == 0) { // Look for 'FROM'

				from_pos = pos + 1;	// Set beginning of from address

				// Flush whitespace, find beginning of from address.
				while (header[from_pos] == ' ') {

					from_pos++;	// Increment position

				}

				pos = from_pos;	// Beginning of sender pos.
				while (header[pos] != ' ' && header[pos] != '\n' &&
					   header[pos] != '\0' && header[pos] != ':' &&
					   header[pos] != ',' && header[pos] != '\r'
						&& pos < header.length()) {

					pos++;	// Iterator

				}

				// Assign "from" email address
				env_from = header.substr(from_pos, pos - from_pos);

			}

		}

		// Search for recipient address
		if (pos > 1) {	// Check if ':' can be preceded by "To"

			// Check for Sender match
			if (strncasecmp((header.substr(pos - 2, pos)).c_str(),
							"To", 2) == 0) {	// Check for 'TO'

				to_pos = pos + 1;	// Set beginning of To: address

				// Flush whitespace
				while (header[to_pos] == ' ') {

					to_pos++;	// Increment position

				}

				pos = to_pos;	// Beginning of recipient pos.
				while (header[pos] != ' ' && header[pos] != '\n' &&
					  header[pos] != '\0' && header[pos] != ':' &&
					  header[pos] != ',' && pos < header.length()) {

					pos++;	// Iterator

				}

				// Assign To: address.
				env_to = header.substr(to_pos, pos - to_pos);

			}

		}

		pos++;	// Increment iterator.

	}

	// Remove angle brackets surrounding addresses if any
	if (env_from[0] == '<' && env_from[env_from.length() - 1] == '>')

		env_from = env_from.substr(1, env_from.length() - 2);

	if (env_to[0] == '<' && env_to[env_to.length() - 1] == '>')

		env_to = env_to.substr(1, env_to.length() - 2);

	// If addresses are empty.
	if (env_from.length() == 0 || env_to.length() == 0) {

		cout << "Error: file envelope addresses incorrect.\n";
		return -1;		// Addresses not found, error.

	}

	// Function to check email syntax.
	if (CheckEmailSyntax(env_from) != 0 ||
		CheckEmailSyntax(env_to) != 0) {

		cout << "Email file address syntax error.\n";
		return -1;

	}

	return 0;	// Return success.

}

/*
 * Check if email address meets all guidelines for syntax as stated by
 * RFC 5322 (section 3.4.1) and RFC 5321.
 * 		Cannot contain more than 254 chars or less than 5 chars
 * 		Cannot have local more than 64 chars or less than 1
 * 		Cannot have local w/ chars besides alpha-numeric and
 * 			~`!.#$%^&\'*{|}-_+=
 * 		Cannot have domain w/ char besides alpha-numeric and
 * 			. (period) or - (hyphen)
 * 		Domain or local cannot start or end w/ . (period)
 * 		Cannot contain consecutive . (periods)
 * 		Domain MAY be in square brackets IF in IP-address form
 * @args:	e-mail address (const string &)
 * @return:	0 (success)
 * - error: -1 (fail)
 *
 *	Address = Local@Domain
 *
 * 	IF NOT(5 < Address length < 254)
 * 		THEN fail
 *
 * 	IF NOT(0 < Local length < 65)
 * 		THEN fail
 *
 *	IF Local chars NOT alpha-numeric OR
 *		~`!.#$%^&\'*{|}-_+="	(one of these symbols)
 *		THEN fail
 *
 *	IF Domain chars NOT alpha-numeric OR
 *		.-		(period OR hyphen)
 *		THEN fail
 *
 *	IF Address first OR last char == '.'
 *		THEN fail
 *
 *	IF Address char == '.' AND
 *		Address char + 1 == '.'
 *		THEN fail
 *
 */
int
CheckEmailSyntax(const string &addr)
{

	int				a_pos = 0;		// Position of '@' char
	string			non_alphanum = "~`!.#$%^&\'*{|}-_+=";
									// String containing all legal alpha-
									// numeric ASCII symbols.
	unsigned int	addr_len = addr.length(),
									// Length of address, must be < 254
					local_len,		// Length of local (before @)
									//   equal to a_pos - 1, must be < 64
					domain_len,		// Length of domain (after @)
									//   equal to addr.length - a_pos
					domain_start;	// Position of 1st char in domain.

	// Must be: 5 < Address length < 254
	if (addr_len > 255 || addr_len < 5) {

		// Illegal address length.
		return -1;

	}

	a_pos = addr.find('@');		// Find position of '@' symbol.

	if (a_pos < 2 || a_pos > 65 ) {

		// Local must be: 0 < local < 65.
		return -1;		// Illegal address.

	}

	local_len = a_pos;
	domain_start = a_pos + 1;
	domain_len = addr_len - a_pos - 1;

	// Verify legality of each char in local
	for (unsigned int i = 0; i < local_len; i++) {

		if (isalnum(addr[i]) == 0) {		// Non-alphanumeric

			// Check if char is legal non-alphanumeric.
			if (non_alphanum.find(addr[i]) == 0) {

				return -1;	// Illegal char in local.

			}

		}

	}

	// Square braces around IP address domain is legal.
	if (addr[domain_start] == '[' && addr[domain_len - 1] == ']') {

		domain_len -= 2;
		++domain_start;

	}

	// Verify legality of each char in domain
	for (unsigned int i = 0; i < domain_len; i++) {

		if (isalnum(addr[i + domain_start]) == 0) {	// Non-alphanumeric

			if (addr[i + domain_start] != '.' &&
				addr[i + domain_start] != '-') {	// Not '.' or '-'

				return -1;	// Illegal char in domain.

			}

		}

	}

	// Verify '.' is not 1st or last char in local.
	if (addr[0] == '.' || addr[local_len - 1] == '.') {

		return -1;	// Illegal address syntax.

	}

	// Verify '.' is not 1st or last char in domain.
	if (addr[domain_start] == '.' || addr[domain_len - 1] == '.') {

		return -1;	// Illegal address syntax.

	}

	// Verify '.' does not appear consecutively.
	for (unsigned int i = 0; i > 0; i = addr.find('.', i + 1)) {

		if (i == (addr.find('.', i + 1) - 1)) {

			return -1;	// Illegal address syntax.
		}

	}

	return 0;	// Valid e-mail address.

}

/*
 * Using ifstream, load hostname settings from configuration file
 * "mailsender.conf" which contains a single line with the format:
 * 		host=<hostname> port=<portnumber> auth=0
 * Authorization type currently disabled.
 * Call sub-method "ParseConfig(...)" to handle string parsing
 * from file.
 * @args:	hostname (string &host),
 * 			port number (int &port),
 * 			authentication type (string &auth)
 * @return:	0 (success)
 *  -error: -1 (file not found: errno, improper format)
 */
int
LoadHost(string &host, int &port, string &auth)
{

	ifstream		fin;	// Input file stream
	string			buf;	// Config file buffer
	int				i;		// Index

	fin.open(ConfigFile.c_str());

	if (fin.fail())

		return -1;		// Config file not found, errno

	getline(fin, buf);

	fin.close();

	// Find hostname

	if ((i = ParseConfig(host, buf, "host=")) == -1)

		return -1;		// File stream parse error

	// Find port number

	if ((i = ParseConfig(port, buf, "port=", i)) == -1)

		return -1;	// port tag not found

	// Find authorization type

	if ((i = ParseConfig(auth, buf, "auth=", i)) == -1)

		return -1;	// auth tag not found

	return 0;		// success

}

/*
 * Parse a segment of buffer, searching for given 'tag' starting at
 * a position, retrieving the following substring.
 * Use <sstream> to convert buffer string to a stream and use '>>'
 * extraction operator to input following string.
 * @args:	found string (string &parsed),
 * 			unparsed buffer (string &buffer),
 * 			tag to search for in buffer (string tag),
 * 			position in buffer to start searching (int pos)
 * @return:	position after found string, from int pos (success)
 *  -error:	-1 (parameter not found)
 */
int
ParseConfig(string &parsed, string &buffer, string tag, int pos)
{

	stringstream	buf_str;

	if ((pos = buffer.find(tag)) == -1)

		return -1;		// tag not found

	else

		pos += tag.length();

	buf_str.str(buffer.substr(pos));
	buf_str >> parsed;

	if (buf_str.bad())	// check bad stream input

		return -1;		// error: check errno

	buf_str.clear();

	return pos + parsed.length();	// return position after string

}

/*
 * ParseConfig: overloaded function
 * Parses an integer value instead of a string.
 * For use w/ port number retrieval.
 * @args:	found int (int &parsed),
 * 			unparsed buffer (string &buffer),
 * 			tag to search for in buffer (string tag),
 * 			position in buffer to start searching (int pos)
 * @return:	position after found int, from int pos (success)
 *  -error:	-1 (parameter not found)
 */
int
ParseConfig(int &parsed, string &buffer, string tag, int pos)
{

	stringstream	buf_str;	// Set string to stream buffer

	if ((pos = buffer.find(tag)) == -1)

		return -1;		// tag not found

	else

		pos += tag.length();

	buf_str.str(buffer.substr(pos));
	buf_str >> parsed;

	if (buf_str.bad())	// check bad stream input

		return -1;		// error: check errno

	buf_str.clear();

	// Return position after integer
	return pos +
		   static_cast<int>(floor(log10(parsed)));
	// Use log-base10 to calc length of integer

}
