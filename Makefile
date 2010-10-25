# Makefile for Mail Sender Program
# Author: Joseph Lee
# CS 300, Assignment #02
# October 25, 2010

CC=g++
LFLAGS=-Wall -g
CFLAGS=$(LFLAGS) -c
SRC=main.cc MailSenderSmtp.cc
OBJ=$(SRC:.cc=.o)
EXEC=mailsender

all: $(EXEC) config

$(EXEC): $(OBJ)
	$(CC) -Wall -g $(OBJ) -o $(EXEC)

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

config:
	$(CC) $(CFLAGS) config.cc -o config

clean:
	rm -rf mailsender config *.o