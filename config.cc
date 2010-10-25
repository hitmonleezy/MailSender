/*
 * Mail Sender Configuration
 * config.cc
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
 * This configuration program is used to set up the file
 * "mailsender.conf" with the appropriate information for the
 * mailsender program to connect to a host.
 * The default values set the relay host to "mailhost.cecs.pdx.edu"
 * and the default SMTP port of 25.
 * The 3rd data member represents the Authentication method used by
 * the particular host. At this point, authentication is disabled and
 * set to the default value of "0".
 * The command-line help switch prints the config_help file that shows
 * how to use the config program.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cctype>

using namespace std;

const string	FileName = "mailsender.conf";
const string	DefaultHost = "mailhost.cecs.pdx.edu";
const string	DefaultPort = "25";
const string	DefaultAuth = "0";
const string	HelpFile = "config_help";

// Check if config help is requested

int HelpCheck(const char *help_req);

int
main(int argc, char **argv) {

	string			host,	// SMTP relay host
					port,	// Port number
					auth;	// Authentication type: "0" for none
	ofstream		fout;

	switch (argc) {

	case 1:
		host = DefaultHost;
		port = DefaultPort;
		auth = DefaultAuth;
		break;

	case 2:
		if (HelpCheck(argv[1]) == 1)
			return -1;		// Help request, config file unmade

		host = argv[1];
		port = DefaultPort;
		auth = DefaultAuth;
		break;

	case 3:
		host = argv[1];
		port = argv[2];
		auth = DefaultAuth;
		break;

	case 4:
		host = argv[1];
		port = argv[2];
		auth = DefaultAuth;	// Authentication disabled
		break;

	default:
		cout << "Error: " << argc << " parameters.\n"
			 << "Type \"config --help\" for help info.\n";
		return -1;

	}

	for (unsigned int i = 0; i < port.length(); i++) {

		if (isdigit(port[i]) == 0) {

			cout << "Error: non-numeric port number.\n";
			return -1;

		}

	}

	fout.open(FileName.c_str());
	fout << "host=" << host << " port=" << port << " auth=" << auth;
	fout.close();

	cout << "Configuration complete.\n";

	return 0;

}

/*
 * Check if command-line argument was help request switch
 * "-help" or "--help" or "-h"
 * Print contents of file "config_help"
 * @args:	cmd-line arg (const char *help_req)
 * @return:	1 if help was requested,
 * 			0 if not.
 */
int
HelpCheck(const char *help_req)
{

	ifstream		fin;	// Input file stream
	char			ch;		// File character buffer

	if (strcmp(help_req, "--help") == 0 ||
		strcmp(help_req, "-h") == 0 ||
		strcmp(help_req, "-help") == 0) {

		fin.open(HelpFile.c_str());

		while(fin.get(ch))		// Print file contents

			cout << ch;

		cout << endl;
		return 1;

	}

	return 0;

}
