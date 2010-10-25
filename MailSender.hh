/*
 * Mail-Sending Program
 * MailSender.hh
 *
 *  Created on: Oct 8, 2010
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

#ifndef MAILSENDER_HH_
#define MAILSENDER_HH_

#include <iostream>
#include <string>

using namespace std;

/*
 * MailSender object, abstract base class
 * @data:		name of email file to send (string Filename) [Private]
 * @methods:	get Filename (string get_filename()) [Protected]
 *
 * This is an abstract base class with a single data member,
 * Filename and it's protected 'get' file.
 * The pure virtual function 'Send' is to be overloaded and
 * implemented in the derived class to send the contents within
 * "filename" via SMTP.
 */
class MailSender
{
  public:

			 MailSender(const string &filename) { Filename = filename; }
			~MailSender() { };

	 // Pure virtual method to send an email (formatted to

	 // RFC 821 specifications) to a relay host (param 1) by

	 // way of SMTP interface.

	virtual int		send(const string &host_to,
						 const string &envelope_from,
						 const string &envelope_to) = 0;

  protected:

	string			get_filename() { return Filename; }

  private:

	string			Filename;

};

#endif /* MAILSENDER_HH_ */
