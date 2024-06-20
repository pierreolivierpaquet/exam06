/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 21:13:12 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/20 13:18:05 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>	// sockaddr_in -> ubuntu
#include <stdbool.h>

/// @brief Represents a client for the server.
typedef struct	s_client
{
	int					fd;
	struct sockaddr_in	client_address;
	char				buffer[BUFSIZ];
}						t_client;

typedef	struct	s_server
{
	int					port;				// Listened port.
	int					fd;					// Socket file descriptor.
	struct sockaddr_in	socket_address;		// 
	t_client			clients[SOMAXCONN];	// Array of client(s).
	fd_set				current_set;		// fd_set data type represents file descriptor sets for the select function. It is actually a bit array.
	fd_set				read_set;			//	Set of file descriptors that have a reading status.
	fd_set				write_set;			//	Set of file descriptors that are ready to be written.
	int					highest_fd;			// Amount of clients within the server.
}						t_server;

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



int	mini_serv(void)
{
	t_server			*server;
	int					i;
	int					new_client_fd;
	struct sockaddr_in	client_address;
	socklen_t			sock_lenght;

	server = NULL;
	get_server(&server);

	while (true)
	{
//		select() function is destructive, so we need to make a copy. current_set serves as a backup of all client file descriptors.
		server->read_set = server->current_set;
		server->write_set = server->current_set;

//		The first parameter is the highest-numbered file descriptor plus one among the sets of file descriptors that you are interested in monitoring for read, write, or exceptional conditions.
//		The function will examine the file descriptor(s) that are ready for reading and writing/to keep an eye on.
		if (select(server->highest_fd + 1,	\
					&server->read_set,		\
					&server->write_set,		\
					NULL,					\
					NULL) < 0)
			continue; // If select() encounters an error, the program must not quit; Instead, it is going to retry to examine the file descriptor(s).

		i = server->fd;
		while (i <= server->highest_fd) // Iterating through all the server file descriptor(s).
		{
			if(FD_ISSET(i, &server->read_set)) // If the 'i' fd is ready for reading.
				{
					if (i == server->fd) // means that a new client connection is required.
					{
						new_client_fd = 0;
						bzero(&client_address, sizeof(client_address));
						sock_lenght = 0;
						new_client_fd = accept(server->fd, \
												(struct sockaddr *)&client_address, \
												&sock_lenght);
					}
					
				}
			i++;
		}

	}
	return (0);
}

int	main(int argc, char **argv)
{
	t_server	*server;

	if (argc != 2 || argv == NULL)
		return (ft_putstring_fd("Wrong number of arguments", STDERR_FILENO),	\
				EXIT_FAILURE);

	server = NULL;
	get_server(&server);
	bzero(server, sizeof(*server));
	convert_port(&server->port, argv[1]);
	if (server->port < 1024 || server->port > 65535)
		return (EXIT_FAILURE);

//	Creates an endpoint for communication and returns a descriptor.
	server->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->fd < 0)
		return (ft_putstring_fd("Fatal error", STDERR_FILENO), EXIT_FAILURE);
	server->highest_fd = server->fd;

	server->socket_address.sin_family = AF_INET;								// Internet address family (TCP, UDP, etc.)
	server->socket_address.sin_addr.s_addr = htonl(INADDR_ANY);					// I think htonl() function call is not mandatory since INADDR_ANY has a value of 0.
	server->socket_address.sin_port = htons(server->port);						// Transforms the port number from little-endian to big-endian.

//	Adding the server's file descriptor to the current_set fd_set.
	FD_SET(server->fd, &server->current_set);									// Includes a particular descriptor fd in fdset.

//	Binding process between the sockaddr_in data and the socket file descriptor.
	if (bind(server->fd,										\
		(const struct sockaddr *)&server->socket_address,		\
		sizeof(server->socket_address)) < 0)
		return (ft_putstring_fd("Fatal error", STDERR_FILENO),	\
				EXIT_FAILURE);	// Binding failed.

//	Listening process.
	if (listen(server->fd, SOMAXCONN) < 0)
		return(ft_putstring_fd("Fatal error", STDERR_FILENO),	\
				EXIT_FAILURE);	// Listening on network failed.

//	Server process.
	mini_serv();

	return (EXIT_SUCCESS);
}

// [X] Check validity of arguments.
// [X] Create a socket (sockaddr_in) for the server itself.
// [X] Bind the socket to network.
// [X] listen for incoming connetions.
// [] accept incoming connections. 