/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 21:13:12 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/19 10:59:02 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in -> ubuntu
#include <stdbool.h>

typedef struct s_client
{
	int		fd;
	char	buffer[1000];
}	t_client;

typedef	struct s_server
{
	t_client clients[1024]; // Array of server client(s).
	fd_set current; // fd_set data type represents file descriptor sets for the select function. It is actually a bit array.
	int	total_clients; // Amount of clients within the server.
}	t_server;

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

int	main(int argc, char **argv)
{
	int	port;
	int	server_fd;
	struct sockaddr_in server_address;	//	 sin_zero -> Padding to make sure that the struct is the same size as the original sockaddr struct.

	static t_server	*server;

	port = 0;
	if (argc != 2 || argv == NULL)
		return (ft_putstring_fd("Wrong number of arguments", STDERR_FILENO), EXIT_FAILURE);
	convert_port(&port, argv[1]);
	if (port < 1024 || port > 65535)
		return (EXIT_FAILURE);

	server_fd = 0;

//	Creates an endpoint for communication and returns a descriptor.
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	return (ft_putstring_fd("Fatal error", STDERR_FILENO), EXIT_FAILURE);

//	Preparing the socketaddr_in struct before binding().

	server->clients[3].fd = 666; // test delete
	bzero(server->clients, sizeof(server->clients));
	bzero(&server_address, sizeof(server_address)); // Clears the struct of any garbage data.

	server_address.sin_family = AF_INET; // Internet address family (TCP, UDP, etc.)
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); // I think htonl() function call is not mandatory since INADDR_ANY has a value of 0.
	server_address.sin_port = htons(port); // Transforms the port number from little-endian to big-endian.

	FD_ZERO(&server->current); // Sets all the fdset bits to 0.
	FD_SET(server_fd, &server->current); // Includes a particular descriptor fd in fdset.

//	Binding process between the sockaddr_in data and the socket file descriptor.
	if (bind(server_fd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0)
		return (ft_putstring_fd("Fatal error", STDERR_FILENO), EXIT_FAILURE); // Binding failed.

//	Listening process.
	if (listen(server_fd, SOMAXCONN) < 0)
		return(ft_putstring_fd("Fatal error", STDERR_FILENO), EXIT_FAILURE); // Listening on network failed.

	return (EXIT_SUCCESS);
}

// [X] Check validity of arguments.
// [X] Create a socket (sockaddr_in) for the server itself.
// [X] Bind the socket to network.
// [X] listen for incoming connetions.
// [] accept incoming connections. 