/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/26 11:33:20 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/26 14:16:53 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct s_client
{
	int					id;
	char				buffer[100000];
	struct sockaddr_in	address;
}	t_client;

// -------------------------------------------------------------------------- //
// GLOBAL VARIABLES

int					server_fd = 0;
int					server_port = 0;
int					highest_fd = 0;
struct sockaddr_in	server_address;
t_client			clients[1024];
int					client_id = 0;
char				receive_buffer[120000];
char				send_buffer[120000];
fd_set				monitored_set;
fd_set				read_set;
fd_set				write_set;

// -------------------------------------------------------------------------- //

void	error(char *message, int fd, int exit_status)
{
	if (message == NULL || fd < 0)
		return ;
	write(fd, message, strlen(message));
	write(fd, "\n", 1);
	if (exit_status != 0)
		exit(exit_status);
	return ;
}

// -------------------------------------------------------------------------- //

void	broadcast(int sender)
{
	int	fd_iterator;

	fd_iterator = server_fd;
	while (fd_iterator <= highest_fd)
	{
		if (fd_iterator != sender && FD_ISSET(fd_iterator, &write_set))
			send(fd_iterator, send_buffer, strlen(send_buffer), 0);
		fd_iterator++;
	}
	bzero(send_buffer, strlen(send_buffer));
	return ;
}

// -------------------------------------------------------------------------- //

void	removeClient(int client_fd)
{
	sprintf(send_buffer, "server: client %d just left\n", clients[client_fd].id);
	broadcast(client_fd);
	bzero(&clients[client_fd], sizeof(*clients));
	FD_CLR(client_fd, &monitored_set);
	close(client_fd);
	return ;
}

// -------------------------------------------------------------------------- //

void	handleData(int client_fd, int bytes_received)
{
	int	i = 0;
	int	j = strlen(clients[client_fd].buffer);

	while (i < bytes_received)
	{
		clients[client_fd].buffer[j] = receive_buffer[i];
		if (clients[client_fd].buffer[j] == '\n')
		{
			clients[client_fd].buffer[j] = '\0';
			sprintf(send_buffer, "client %d: %s\n", clients[client_fd].id, clients[client_fd].buffer);
			broadcast(client_fd);
			bzero(clients[client_fd].buffer, strlen(clients[client_fd].buffer));
			j = -1;
		}
		i++;
		j++;
	}
	return ;
}

// -------------------------------------------------------------------------- //

void	receiveData(int client_fd)
{
	int	bytes_received = 0;
	int	buffersize = sizeof(receive_buffer);
	bytes_received = recv(client_fd, receive_buffer, buffersize, 0);
	if (bytes_received <= 0)
		removeClient(client_fd);
	else
		handleData(client_fd, bytes_received);
	return ;
}

// -------------------------------------------------------------------------- //

int	acceptClient(void)
{
	int					new_client_fd = 0;
	struct sockaddr_in	address;
	socklen_t			lenght;

	bzero(&address, sizeof(address));
	lenght = sizeof(address);
	new_client_fd = accept(server_fd, (struct sockaddr *)&address, &lenght);
	if (new_client_fd < 0)
		return (-1);
	FD_SET(new_client_fd, &monitored_set);
	if (new_client_fd > highest_fd)
		highest_fd = new_client_fd;
	clients[new_client_fd].id = client_id++;
	clients[new_client_fd].address = address;
	sprintf(send_buffer,													\
			"server: client %d just arrived\n",								\
			clients[new_client_fd].id);
	broadcast(new_client_fd);
	return (0);
}

// -------------------------------------------------------------------------- //

void	miniServer(void)
{
	int		fd_iterator = 0;
	while (true)
	{
		read_set = monitored_set;
		write_set = monitored_set;
		if (select(highest_fd + 1, &read_set, &write_set, NULL, NULL) < 0)
			continue;
		fd_iterator = server_fd;
		while (fd_iterator <= highest_fd)
		{
			if (FD_ISSET(fd_iterator, &read_set))
			{
				if (fd_iterator == server_fd)
				{
					if (acceptClient() < 0)
						continue ;
				}
				else
					receiveData(fd_iterator);
				break ;
			}
			fd_iterator++;
		}
	}
	return ;
}


// -------------------------------------------------------------------------- //

void	createSocket(void)
{
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(server_port);
	server_address.sin_family = AF_INET;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (server_fd < 0 ||													\
		bind(server_fd,														\
			(const struct sockaddr *)&server_address,						\
			sizeof(server_address)) < 0 ||									\
		listen(server_fd, SOMAXCONN) < 0)
		error("Fatal error", STDERR_FILENO, EXIT_FAILURE);

	FD_SET(server_fd, &monitored_set);
	highest_fd = server_fd;
	return ;
}

// -------------------------------------------------------------------------- //

void	convertPort(char *av)
{
	if (av == NULL)
		return ;
	server_port = atoi(av);
	if (server_port < 1024 || server_port > 65535)
		error("Fatal error", STDERR_FILENO, EXIT_FAILURE);
	return ;
}

// -------------------------------------------------------------------------- //

int	main(int argc, char **argv)
{
	if (argc != 2)
		error("Wrong number of arguments", STDERR_FILENO, EXIT_FAILURE);
	else
	{
		convertPort(argv[1]);
		createSocket();
		miniServer();
	}
	return (EXIT_SUCCESS);
}
