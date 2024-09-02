#
# $Id: Makefile,v 1.1 2009/09/16 03:58:29 mit Exp mit $
#
SRC = lib.c libg.c daemon.c cli1.c times.c cli1g.c timesg.c echoc.c \
	echocg.c echos1.c \
	echos1g.c echos2.c echos2g.c echos2gt.c denc.c dencg.c dens.c densg.c \
	js.c jc.c jsg.c jcg.c jcgf.c jsgnf.c jsgnm.c jcgnm.c \
	jcgnmf.c echos2gp.c updtime.c udptimed.c uvecho.c kqe.c line.c lcine.c newline.c newlcine.c

EXE = cli1 times cli1g timesg denc dens dencg densg echoc echocg echos1 \
	echos1g echos2 echos2g echos2gt \
	js jc jsg jcg jcgf jsgn jcgn jcgnf jsgnm jcgnm jcgnmf \
	echos2gp udptime udptimed uvecho kqe surl jsgnmf sigio line lcine newline newlcine


ML2 = lib.c daemon.c
ML3 = libg.c daemon.c
LIBS = #-lsocket -lnsl
LUV = -L/usr/local/lib -luv
#CC = gcc -g
#CC = gcc -Wall -std=c99 -Wextra -Werror
#CC = gcc -Wall -std=c99 -Wextra
CC = clang -g -Wall -std=c99 -Wextra


all0: cli1 times cli1g timesg denc dens dencg densg echoc echocg echos1 \
	echos1g echos2 echos2g echos2gt echos2gp udptime udptimed  \
	filelock filenolock sigio

all1: js jc jsg jcg jcgf jsgn jcgn jcgnf jsgnm jcgnm jcgnmf

cli1: cli1.c $(ML2)
	$(CC) -o cli1 cli1.c $(ML2) $(LIBS)

times: times.c $(ML3)
	$(CC) -o times times.c $(ML2) $(LIBS)

cli1g: cli1g.c $(ML3)
	$(CC) -o cli1g cli1g.c $(ML3) $(LIBS)

timesg: timesg.c $(ML3)
	$(CC) -o timesg timesg.c $(ML3) $(LIBS)

echoc: echoc.c $(ML)
	$(CC) -o echoc echoc.c $(ML2) $(LIBS)

echocg: echocg.c $(ML3)
	$(CC) -o echocg echocg.c $(ML3) $(LIBS)

echos1: echos1.c $(ML2)
	$(CC) -o echos1 echos1.c $(ML2) $(LIBS)

echos1g: echos1g.c $(ML3)
	$(CC) -o echos1g echos1g.c $(ML3) $(LIBS)

echos2: echos2.c $(ML2)
	$(CC) -o echos2 echos2.c $(ML2) $(LIBS)

echos2g: echos2g.c $(ML3)
	$(CC) -o echos2g echos2g.c $(ML3) $(LIBS)

echos2gp: echos2gp.c $(ML3)
	$(CC) -o echos2gp echos2gp.c $(ML3) $(LIBS)

echos2gt: echos2gt.c $(ML3)
	$(CC) -lpthread -o echos2gt echos2gt.c $(ML3) $(LIBS)

denc: denc.c $(ML)
	$(CC) -o denc denc.c $(ML2) $(LIBS)

dencg: dencg.c $(ML3)
	$(CC) -o dencg dencg.c $(ML3) $(LIBS)

dens: dens.c $(ML)
	$(CC) -o dens dens.c $(ML2) $(LIBS)

densg: densg.c $(ML3)
	$(CC) -o densg densg.c $(ML3) $(LIBS)

js: js.c $(ML2)
	$(CC) -o js js.c $(ML2) $(LIBS)

jc: jc.c $(ML2)
	$(CC) -o jc jc.c $(ML2) $(LIBS)

jsg: jsg.c $(ML3)
	$(CC) -o jsg jsg.c $(ML3) $(LIBS)

jcg: jcg.c $(ML3)
	$(CC) -o jcg jcg.c $(ML3) $(LIBS)

jcgf: jcgf.c $(ML3)
	$(CC) -o jcgf jcgf.c $(ML3) $(LIBS)

jsgn: jsgn.c $(ML3)
	$(CC) -o jsgn jsgn.c $(ML3) $(LIBS)

jcgn: jcgn.c $(ML3)
	$(CC) -o jcgn jcgn.c $(ML3) $(LIBS)

jcgnf: jcgnf.c $(ML3)
	$(CC) -o jcgnf jcgnf.c $(ML3) $(LIBS)

jsgnm: jsgnm.c $(ML3)
	$(CC) -o jsgnm jsgnm.c $(ML3) $(LIBS)

jcgnm: jcgnm.c $(ML3)
	$(CC) -o jcgnm jcgnm.c $(ML3) $(LIBS)

jcgnmf: jcgnmf.c $(ML3)
	$(CC) -o jcgnmf jcgnmf.c $(ML3) $(LIBS)

jsgnmf: jsgnmf.c $(ML3)
	$(CC) -o jsgnmf jsgnmf.c $(ML3) $(LIBS)

udptime: udptime.c $(ML2)
	$(CC) -o udptime udptime.c $(ML2) $(LIBS)

udptimed: udptimed.c $(ML2)
	$(CC) -o udptimed udptimed.c $(ML2) $(LIBS)

filenolock: filelock.c
	$(CC) -o filenolock -DNOLOCK filelock.c

filelock: filelock.c
	$(CC) -o filelock  filelock.c

ohcecg: ohcecg.c $(ML3) 
	$(CC) -o ohcecg ohcecg.c $(ML3) $(LIBS)

ohcesg: ohcesg.c $(ML3) 
	$(CC) -o ohcesg ohcesg.c $(ML3) $(LIBS)

kqe: kqe.c $(ML3)
	$(CC) -o kqe kqe.c $(ML3) $(LIBS)

uvecho: uvecho.c $(ML3) 
	$(CC) -I/usr/local/include -o uvecho uvecho.c $(ML3) $(LIBS) $(LUV)

surl: surl.c $(ML3) 
	$(CC)  -o surl surl.c $(ML3) $(LIBS) -lssl

line: line.c $(ML3)
	$(CC) -o line line.c $(ML3) $(LIBS)

lcine: lcine.c $(ML3)
	$(CC) -o lcine lcine.c $(ML3) $(LIBS)

newline: newline.c $(ML3)
	$(CC) -o newline newline.c $(ML3) $(LIBS)

newlcine: newlcine.c $(ML3)
	$(CC) -o newlcine newlcine.c $(ML3) $(LIBS)

clean:
	rm -f *.o
	rm -rf *.dSYM
	rm -f $(EXE)
