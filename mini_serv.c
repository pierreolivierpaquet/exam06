/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 21:13:12 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/19 14:10:25 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>	// sockaddr_in -> ubuntu
#include <stdbool.h>

typedef struct	s_client
{
	int			fd;
	char		buffer[BUFSIZ];
}				t_client;

typedef	struct	s_server
{
	t_client			clients[SOMAXCONN];	// Array of server client(s).
	int					fd;				// Own file descriptor (server).
	int					port;			// Listened port.
	struct sockaddr_in	socket_address;
	fd_set				current;		// fd_set data type represents file descriptor sets for the select function. It is actually a bit array.
	int					total_clients;	// Amount of clients within the server.
}				t_server;

int	ft_putstring_fd(char *string, int fd)
{
	if (string == NULL || fd < 0)
		return (-1);
	write(fd, string, strlen(string));
	write(fd, "\n", 1);
	return (0);
}

void	convert_port(int *port, char *arg)
{
	if (port == NULL)
		return ;
	*port = atoi(arg);
	return;
}

/// @returns A pointer to the server.
void	get_server(t_server **get)
{
	static t_server	server[1];

	*get = server;
	return ;
}

int	main(int argc, char **argv)
{
	t_server *server = NULL;
	get_server(&server);

	bzero(server, sizeof(*server));
	if (argc != 2 || argv == NULL)
		return (ft_putstring_fd("Wrong number of arguments", STDERR_FILENO), EXIT_FAILURE);
	convert_port(&server->port, argv[1]);
	if (server->port < 1024 || server->port > 65535)
		return (EXIT_FAILURE);

//	Creates an endpoint for communication and returns a descriptor.
	server->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->fd < 0)
	return (ft_putstring_fd("Fatal error", STDERR_FILENO), EXIT_FAILURE);
	server->total_clients = server->fd;

	server->socket_address.sin_family = AF_INET;								// Internet address family (TCP, UDP, etc.)
	server->socket_address.sin_addr.s_addr = htonl(INADDR_ANY);					// I think htonl() function call is not mandatory since INADDR_ANY has a value of 0.
	server->socket_address.sin_port = htons(server->port);						// Transforms the port number from little-endian to big-endian.

	FD_SET(server->fd, &server->current);										// Includes a particular descriptor fd in fdset.

//	Binding process between the sockaddr_in data and the socket file descriptor.
	if (bind(server->fd, (const struct sockaddr *)&server->socket_address, sizeof(server->socket_address)) < 0)
		return (ft_putstring_fd("Fatal error", STDERR_FILENO), EXIT_FAILURE);	// Binding failed.

//	Listening process.
	if (listen(server->fd, SOMAXCONN) < 0)
		return(ft_putstring_fd("Fatal error", STDERR_FILENO), EXIT_FAILURE);	// Listening on network failed.

	return (EXIT_SUCCESS);
}

// [X] Check validity of arguments.
// [X] Create a socket (sockaddr_in) for the server itself.
// [X] Bind the socket to network.
// [X] listen for incoming connetions.
// [] accept incoming connections. 