/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 21:13:12 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/22 16:07:38 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h> // select(); fd_set -> ubuntu
#include <netinet/in.h>	// sockaddr_in -> ubuntu
#include <stdbool.h>

/// @brief Represents a client for the server.
typedef struct	s_client
{
	int					id;
	struct sockaddr_in	client_address;
	char				buffer[1024];
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
	int					client_id;			// Client identification.
	char				send_buf[BUFSIZ];
	char				receive_buf[BUFSIZ];
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
	int					bytes_received;
	int					j;
	int					k;

	server = NULL;
	get_server(&server);

	printf("ENTERING THE INFINTE LOOP\n");
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
		{
			continue; // If select() encounters an error, the program must not quit; Instead, it is going to retry to examine the file descriptor(s).
		}

		i = server->fd;
		while (i <= server->highest_fd) // Iterating through all the server file descriptor(s).
		{
			if(FD_ISSET(i, &server->read_set)) // If the 'i' fd is ready for reading.
			{
				if (i == server->fd) // Means that a new client connection is required.
				{
					new_client_fd = 0;
					bzero(&client_address, sizeof(client_address));
					sock_lenght = 0;
					new_client_fd = accept(server->fd, \
											(struct sockaddr *)&client_address, \
											&sock_lenght);
					if (new_client_fd > 0)
					{
						if (new_client_fd > server->highest_fd)
							server->highest_fd = new_client_fd;
						server->clients[new_client_fd].id = server->client_id;
						server->client_id++;
						FD_SET(new_client_fd, &server->current_set);		// Adds the new client file descriptor to the monitored set.
						sprintf(server->send_buf, "server: client %d just arrived\n", server->clients[new_client_fd].id);
						printf("%s", server->send_buf);
					}
					else
						continue;
				}
				else // Means that a message is sent.
				{
					bytes_received = recv(i, server->receive_buf, sizeof(server->receive_buf), 0);
					if (bytes_received <= 0)
					{
						sprintf(server->send_buf, "server: client %d just left\n", server->clients[i].id);
						// ADD FUNCTION THAT SENDS TO EVERY CLIENT
						printf("%s", server->send_buf);
						FD_CLR(i, &server->current_set); // Deletes the file descriptor from the monitored set.
						close(i); // Closes the file descriptor.
						bzero(&server->clients[i], sizeof(server->clients[i])); // Deletes the data stored to the client struct.
					}
					else
					{
						j = 0; // To parse the received message.
						k = strlen(server->clients[i].buffer); // To navigate into the client message buffer.
						while (j < bytes_received)
						{
							server->clients[i].buffer[k] = server->receive_buf[j]; // Copies the content of the received buffer to the client's message.
							if (server->clients[i].buffer[k] == '\n')
							{
								server->clients[i].buffer[k] = '\0';
								sprintf(server->send_buf, "client %d: %s\n", server->clients[i].id, server->clients[i].buffer);
								// ADD FUNCTION THAT SENDS TO EVERY CLIENT
								printf("%s", server->send_buf);
								bzero(server->clients[i].buffer, strlen(server->clients[i].buffer));
								k = -1;
							}
							j++;
							k++;
						}
					}
				}
				break;
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
