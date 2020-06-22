// SERBOI FLOREA-DAN 325CB
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char* get_ip(char *name)
{
	char* buf = NULL;
	struct addrinfo hints, *p = NULL;
	// set hints
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	// get address
	int r = getaddrinfo(name, "http", &hints, &p);
	if (r < 0) {
		fprintf(stderr, "%s", gai_strerror(r));
	}
	// store the IPV4 address in buf
	buf = calloc(16, sizeof(char));
	// find IPV4 address
	while (p) {
		if (p->ai_family == AF_INET) {
			struct sockaddr_in *in = (struct sockaddr_in *)p->ai_addr;
			inet_ntop(p->ai_family, &in->sin_addr, buf, 16);
			break;
		}
		p = p->ai_next;
	}
	// free allocated data
	freeaddrinfo(p);
	// return the IPV4 address
	return buf;
}

char *compute_get_request(char *host, char *url, char *query_params,
							char *cookie, char* jwt)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));
	// write the method name, URL, request params (if any) and protocol type
	if (query_params != NULL) {
		sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
	} else {
		sprintf(line, "GET %s HTTP/1.1", url);
	}
	compute_message(message, line);
	// add the host
	sprintf(line, "Host: %s", host);
	compute_message(message, line);
	// add headers and/or cookies
	if (jwt != NULL) {
		sprintf(line, "Authorization: Bearer %s", jwt);
		compute_message(message, line);
	}
	if (cookie != NULL) {
		sprintf(line, "Cookie: %s", cookie);
		compute_message(message, line);
	}
	// add final new line
	compute_message(message, "");
	free(line);
	return message;
}

char *compute_post_request(char *host, char *url, char* content_type, 
						   char *body_data, char *cookie, char* jwt)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));
	char *body_data_buffer = calloc(LINELEN, sizeof(char));
	// write the method name, URL and protocol type
	sprintf(line, "POST %s HTTP/1.1", url);
	compute_message(message, line);
	// add the host
	sprintf(line, "Host: %s", host);
	compute_message(message, line);
	// add necessary headers (Content-Type and Content-Length)
	sprintf(line, "Content-Type: %s", content_type);
	compute_message(message, line);
	int content_length = 0;
	content_length += strlen(body_data);
	sprintf(line, "Content-Length: %d", content_length);
	compute_message(message, line);
	// add cookie
	if (jwt != NULL) {
		sprintf(line, "Authorization: Bearer %s", jwt);
		compute_message(message, line);
	}
	if (cookie != NULL) {
		sprintf(line, "Cookie: %s", cookie);
		compute_message(message, line);
	}
	// add new line at end of header
	compute_message(message, "");
	// add the actual payload data
	memset(line, 0, LINELEN);
	strcat(body_data_buffer, body_data);
	compute_message(message, body_data_buffer);
	free(line);
	free(body_data_buffer);
	return message;
}

char *compute_delete_request(char *host, char *url, 
							char *cookie, char* jwt)
{
	char *message = calloc(BUFLEN, sizeof(char));
	char *line = calloc(LINELEN, sizeof(char));
	// write the method name, URL and protocol type
	sprintf(line, "DELETE %s HTTP/1.1", url);
	compute_message(message, line);
	// add the host
	sprintf(line, "Host: %s", host);
	compute_message(message, line);
	// add headers and/or cookies
	if (jwt != NULL) {
		sprintf(line, "Authorization: Bearer %s", jwt);
		compute_message(message, line);
	}
	if (cookie != NULL) {
		sprintf(line, "Cookie: %s", cookie);
		compute_message(message, line);
	}
	// add final new line
	compute_message(message, "");
	free(line);
	return message;
}

void reg() {
	int sockfd;
	char *message;
	char *response;
	char buffer[BUFLEN];
	// reads username from stdin
	printf("username=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	char username[BUFLEN];
	memset(username, 0, BUFLEN);
	// we take care to copy without newline character
	memcpy(username, buffer, strlen(buffer) - 1);            
	// reads password from stdin
	printf("password=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	char password[BUFLEN];
	memset(password, 0, BUFLEN);
	// we take care to copy without newline character
	memcpy(password, buffer, strlen(buffer) - 1);         
	// the JSON payload with user and password is built
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "username", username);
	json_object_set_string(root_object, "password", password);
	serialized_string = json_serialize_to_string_pretty(root_value);
	// we send message and free it after
	message = 
	compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
	"/api/v1/tema/auth/register", "application/json", serialized_string, 
	NULL, NULL);
	char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
	// open connection
	sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);    
	free(ip);        
	free(message);
	// we receive the answer and after we display it(and free it)
	response = receive_from_server(sockfd);
	// close connection
	close_connection(sockfd);
	puts(response);
	free(response);            
	// free JSON payload
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
}

void login(char **login_cookie) {
	int sockfd;
	char *message;
	char *response;
	char buffer[BUFLEN];
	if(*login_cookie) {
		fprintf(stderr, "Already logged in !\n");
		return;
	} 
	else {
		// reads username from stdin
		printf("username=");
		memset(buffer, 0, BUFLEN);
		fgets(buffer, BUFLEN - 1, stdin);
		char username[BUFLEN];
		memset(username, 0, BUFLEN);
		// we take care to copy without newline character
		memcpy(username, buffer, strlen(buffer) - 1);            
		// reads password from stdin
		printf("password=");
		memset(buffer, 0, BUFLEN);
		fgets(buffer, BUFLEN - 1, stdin);
		char password[BUFLEN];
		memset(password, 0, BUFLEN);            
		// we take care to copy without newline character
		memcpy(password, buffer, strlen(buffer) - 1);
		 // the JSON payload with user and password is built
		JSON_Value *root_value = json_value_init_object();
		JSON_Object *root_object = json_value_get_object(root_value);
		char *serialized_string = NULL;
		json_object_set_string(root_object, "username", username);
		json_object_set_string(root_object, "password", password);
		serialized_string = json_serialize_to_string_pretty(root_value);
		// we send message and free it after
		message = 
		compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
			"/api/v1/tema/auth/login", "application/json", serialized_string, 
			NULL, NULL);
		char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
		// open connection
		sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		free(ip);
		free(message);
		// we receive the answer and after we display it
		response = receive_from_server(sockfd);
		// close connection
		close_connection(sockfd);
		puts(response);
		// if we receive a response with the login cookie, we store it
		if(strstr(response, "200 OK")) {
			char* start = strstr(response, "Cookie: ");
			char* end = strstr(response, "Date");
			*login_cookie = calloc( end - 2 - start - 8 + 1, 1);
			memcpy(*login_cookie, start + 8, end - 2 - (start + 8));
		}
		free(response);            
		// free JSON payload
		json_free_serialized_string(serialized_string);
		json_value_free(root_value);
	}
}

void enter_library(char **login_cookie, char **jwt) {
	int sockfd;
	char *message;
	char *response;
	// we send message and free it after
	message = 
	compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
		"/api/v1/tema/library/access", NULL, *login_cookie, NULL);
	char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
	// open connection
	sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	free(ip);
	free(message);
	// we receive the answer and after we display it
	response = receive_from_server(sockfd);
	// close connection
	close_connection(sockfd);
	puts(response);
	// if we receive a response with the JWT, we store it
	unsigned int token_length = 0;
	char* token = NULL;
	token = strstr(response, "token\":\"");
	if(token) {
		// jump to the token content
		token += 8;
		// find token length
		while(token[token_length] != '\"') {
			token_length++;
		}
		// copy token content
		*jwt = calloc(token_length + 1, sizeof(char));
		memcpy(*jwt, token, token_length);    
	} else {
		fprintf(stderr, "\nDidn't receive JWT !\n");
	}
	free(response);
}

void get_books(char **jwt) {
	int sockfd;
	char *message;
	char *response;
	// we send message and free it after
	message = 
	compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
		"/api/v1/tema/library/books", NULL, NULL, *jwt);
	char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
	// open connection
	sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	free(ip);
	free(message);
	// we receive the answer and after we display it
	response = receive_from_server(sockfd);
	// close connection
	close_connection(sockfd);
	puts(response);         
	free(response);
}

void get_book(char **jwt) {
	int sockfd;
	char *message;
	char *response;
	char buffer[BUFLEN];
	// reads id from stdin
	printf("id=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	// we take the characters without newline
	buffer[strlen(buffer) - 1] = '\0';
	// we build the access route to the requested book
	char access_route[BUFLEN];
	sprintf(access_route, "/api/v1/tema/library/books/%s", buffer);
	// we send message and free it after
	message = 
	compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
		access_route, NULL, NULL, *jwt);
	char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
	// open connection
	sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	free(ip);
	free(message);
	// we receive the answer and after we display it
	response = receive_from_server(sockfd);
	// close connection
	close_connection(sockfd);
	puts(response);
	free(response);
}

void add_book(char **jwt) {
	int sockfd;
	char *message;
	char *response;
	char buffer[BUFLEN];
	// reads title from stdin
	printf("title=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	char title[BUFLEN];
	memset(title, 0, BUFLEN);
	// we take the characters without newline
	memcpy(title, buffer, strlen(buffer) - 1);
	// reads author from stdin
	printf("author=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	char author[BUFLEN];
	memset(author, 0, BUFLEN);
	// we take the characters without newline
	memcpy(author, buffer, strlen(buffer) - 1);
	// reads genre from stdin
	printf("genre=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	char genre[BUFLEN];
	memset(genre, 0, BUFLEN);
	// we take the characters without newline
	memcpy(genre, buffer, strlen(buffer) - 1);
	// reads publisher from stdin
	printf("publisher=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	char publisher[BUFLEN];
	memset(publisher, 0, BUFLEN);
	// we take the characters without newline
	memcpy(publisher, buffer, strlen(buffer) - 1);
	// reads page count from stdin
	printf("page_count=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	// we take the characters without newline
	buffer[strlen(buffer) - 1] = '\0';
	long page_count = strtol(buffer, NULL, 10);
	if(page_count <= 0) {
		fprintf(stderr, "You didn't insert a valid number for page count !\n");
		return;
	}
	// we build JSON payload
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "title", title);
	json_object_set_string(root_object, "author", author);
	json_object_set_string(root_object, "genre", genre);
	json_object_set_number(root_object, "page_count", page_count);
	json_object_set_string(root_object, "publisher", publisher);
	serialized_string = json_serialize_to_string_pretty(root_value);
	// we send message and free it after
	message = 
	compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
		"/api/v1/tema/library/books", "application/json", serialized_string, 
		NULL, *jwt);
	char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
	// open connection
	sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	free(ip);
	free(message);
	// we receive the answer and after we display it
	response = receive_from_server(sockfd);
	// close connection
	close_connection(sockfd);
	puts(response);
	free(response);            
	// we free JSON payload
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
}

void delete_book(char **jwt) {
	int sockfd;
	char *message;
	char *response;
	char buffer[BUFLEN];
	// reads id from stdin
	printf("id=");
	memset(buffer, 0, BUFLEN);
	fgets(buffer, BUFLEN - 1, stdin);
	// we take the characters without newline
	buffer[strlen(buffer) - 1] = '\0';
	// we build the access route to the requested book
	char access_route[BUFLEN];
	sprintf(access_route, "/api/v1/tema/library/books/%s", buffer);
	// we send message and free it after
	message = 
	compute_delete_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
		access_route, NULL, *jwt);
	char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
	// open connection
	sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	free(ip);
	free(message);
	// we receive the answer and after we display it
	response = receive_from_server(sockfd);
	// close connection
	close_connection(sockfd);
	puts(response);
	free(response);
}

void logout(char **login_cookie, char **jwt) {
	int sockfd;
	char *message;
	char *response;
	// we can disconnect only if we are connected
	if(*login_cookie) {
		// we send message and free it after
		message = 
		compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
			"/api/v1/tema/auth/logout", NULL, *login_cookie, NULL);
		char* ip = get_ip("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com");
		// open connection
		sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
		send_to_server(sockfd, message);
		free(ip);
		free(message);
		// we receive the answer and after we display it
		response = receive_from_server(sockfd);
		// close connection
		close_connection(sockfd);
		puts(response);
		free(response);
		// we drop the cookie and token held
		free(*login_cookie);
		free(*jwt);
		*login_cookie = NULL;
		*jwt = NULL;
	}
	else {
		fprintf(stderr, "Not logged in, you cannot log out !\n");
	}
}