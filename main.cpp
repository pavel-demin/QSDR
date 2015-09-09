#include <QtGui/QApplication>
#include <QtCore/QCoreApplication>
#include <getopt.h>
#include "qsdrimpl.h"
#include "lib.h"
#include "trx.h"
#include "config.h"

void help(char *prog)
{
    printf("usage: %s [-h] [-v debugLevel] [-s] [-g serverIP] [-p serverPort] [-k key]"
	    "\n\n", 
	    prog);
    printf("  -h                     display this help\n");
    printf("  -s                     start server only\n");
    printf("  -g  <server address>   start GUI, server is remote\n");
    printf("  -p  <server port>      server port (default 51400)\n");
    printf("  -k  <key>   			 access key\n");
    printf("  -v  <debugLevel>       debug output\n");

    exit(0);
}

int main(int argc, char *argv[])
{

	int i,ret;
	bool server = false;
	char *serverIP = NULL;
	char *key = NULL;
	int port = TRX_PORT;

	while ((i = getopt(argc, argv, "+hsp:k:g:v:")) != EOF) {
		switch (i) {
			case 'v':
				debugLevel = atoi(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 's':
				server = true;
				break;
			case 'g':
				serverIP = optarg;
				break;
			case 'k':
				key = optarg;
				break;
			case 'h':
			default:
				help(argv[0]);
				break;
		}
	}


	if (server) {
		QCoreApplication a(argc, argv);
		Trx *trx = new Trx(port, key);
		ret = a.exec();
		delete trx;
	} else {
		QApplication a(argc, argv);
		QSdrImpl *w=NULL;
		w = new QSdrImpl(serverIP, port, key);
		w->show();
		ret = a.exec();
		delete w;
	}

	return ret;
}
