// SERBOI FLOREA-DAN 325CB
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"



int main(int argc, char *argv[])
{

	char buffer[BUFLEN];
	// in this variable we store the login cookie received
	char* login_cookie = NULL;
	// in this variable we store the authorization token received
	char* jwt = NULL;

	while (1) {
		// reads command from stdin
		memset(buffer, 0, BUFLEN);
		fgets(buffer, BUFLEN - 1, stdin);

		if (strcmp(buffer, "register\n") == 0) {
			reg(&login_cookie);
		}
		if (strcmp(buffer, "login\n") == 0) {
		   login(&login_cookie);
		}
		if (strcmp(buffer, "enter_library\n") == 0) {
			enter_library(&login_cookie, &jwt);
		}
		if (strcmp(buffer, "get_books\n") == 0) {
			get_books(&jwt);
		}
		if (strcmp(buffer, "get_book\n") == 0) {
			get_book(&jwt);
		}
		if (strcmp(buffer, "add_book\n") == 0) {
			add_book(&jwt);
		}
		if (strcmp(buffer, "delete_book\n") == 0) {
			delete_book(&jwt);
		}
		if (strcmp(buffer, "logout\n") == 0) {
			logout(&login_cookie, &jwt);
		}
		if (strcmp(buffer, "exit\n") == 0) {
			break;
		}
	}

	// we free what may not be freed
	if(login_cookie) {
		free(login_cookie);
	}
	if(jwt) {
		free(jwt);
	}

	return 0;
}
