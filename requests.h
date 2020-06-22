#include "parson.h"
#ifndef _REQUESTS_
#define _REQUESTS_

// Receives hostname and returns IPV4 address
char *get_ip(char *name);

// computes and returns a GET request string (query_params
// cookie and jwt can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
						  char *cookie, char *jwt);

// computes and returns a POST request string
// (cookie and jwt can be NULL if not needed)
char *compute_post_request(char *host, char *url, char *content_type,
						   char *body_data, char *cookie, char *jwt);

// computes and returns a DELETE request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_delete_request(char *host, char *url,
							 char *cookie, char *jwt);

// client registration
void reg();

// client login
void login(char **login_cookie);

// client authorization
void enter_library(char **login_cookie, char **jwt);

// displays summary information about books in the library
void get_books(char **jwt);

// displays complete information about a certain book
void get_book(char **jwt);

// adds a certain book in the library
void add_book(char **jwt);

// deletes a certain book from the library
void delete_book(char **jwt);

// client logout
void logout(char **login_cookie, char **jwt);

#endif
