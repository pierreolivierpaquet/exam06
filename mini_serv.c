/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 19:20:31 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/24 20:41:36 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h> // strlen()
#include <unistd.h> // write()
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct	s_client
{
	int					id;
	struct sockaddr_in	address;
	char				buffer[900000];
}	t_client;

typedef struct	s_server
{
	int					fd;
	int					port;
	struct sockaddr_in	address;
	int					client_id;
	t_client			clients[SOMAXCONN];
	int					highest_fd;
	char				receive_buffer[1000000];
	char				send_buffer[1000000];
	fd_set				monitored_set;
	fd_set				read_set;
	fd_set				write_set;
}	t_server;

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

void	getServer(t_server **get)
{
	static t_server	server;

	if (get == NULL)
		return ;
	*get = &server;
	return ;
}

// -------------------------------------------------------------------------- //

void	convertPort(char *av)
{
	t_server	*server;

	if (av == NULL)
		return ;
	server = NULL;
	getServer(&server);
	server->port = atoi(av);
	if (server->port < 1024 || server->port > 65535)
		error("Fatal error", STDERR_FILENO, EXIT_FAILURE);
	return ;
}

// -------------------------------------------------------------------------- //

void	broadcast(int sender)
{
	t_server	*server;
	int			fd_iterator;

	server = NULL;
	getServer(&server);

	fd_iterator = server->fd;
	while (fd_iterator <= server->highest_fd)
	{
		if (FD_ISSET(fd_iterator, &server->write_set) && fd_iterator != sender)
		{
			send(fd_iterator,												\
				server->send_buffer,										\
				strlen(server->send_buffer),								\
				0);
		}
		fd_iterator++;
	}
	bzero(server->send_buffer, strlen(server->send_buffer));
	return ;
}

// -------------------------------------------------------------------------- //

void	removeClient(int client_fd)
{
	t_server	*server;

	server = NULL;
	getServer(&server);

	sprintf(server->send_buffer,											\
			"server: client %d just left\n",								\
			server->clients[client_fd].id);
	broadcast(client_fd);
	FD_CLR(client_fd, &server->monitored_set);
	close(client_fd);
	bzero(&server->clients[client_fd], sizeof(*server->clients));
	return ;
}

// -------------------------------------------------------------------------- //

void	handleData(int client_fd, int bytes_received)
{
	t_server	*server;
	int			i;
	int			j;

	server = NULL;
	getServer(&server);
	i = 0;
	j = strlen(server->clients[client_fd].buffer);
	while (i < bytes_received)
	{
		server->clients[client_fd].buffer[j] = server->receive_buffer[i];
		if (server->clients[client_fd].buffer[j] == '\n')
		{
			server->clients[client_fd].buffer[j] = '\0';
			sprintf(server->send_buffer,									\
					"client %d: %s\n",										\
					server->clients[client_fd].id,							\
					server->clients[client_fd].buffer);
			broadcast(client_fd);
			bzero(server->clients[client_fd].buffer,						\
					strlen(server->clients[client_fd].buffer));
			j = -1;
		}
		i++;
		j++;
	}
	return ;
}

// -------------------------------------------------------------------------- //

void	receiveData(int	client_fd)
{
	t_server	*server;
	int			bytes_received;

	server = NULL;
	getServer(&server);
	bytes_received = 0;
	bzero(server->receive_buffer, sizeof(server->receive_buffer));
	bytes_received = recv(client_fd, server->receive_buffer, sizeof(server->receive_buffer), 0);
	if (bytes_received <= 0)
	{
		removeClient(client_fd);
	}
	else
	{
		handleData(client_fd, bytes_received);
	}
	return ;
}

// -------------------------------------------------------------------------- //

int	acceptClient(void)
{
	t_server			*server;
	int					new_client_fd;
	struct sockaddr_in	address;
	socklen_t			lenght;

	server = NULL;
	getServer(&server);
	bzero(&address, sizeof(address));
	lenght = sizeof(address);
	new_client_fd = accept(server->fd,										\
							(struct sockaddr *)&address,					\
							&lenght);
	if (new_client_fd < 0)
		return (-1);
	server->highest_fd = (new_client_fd > server->highest_fd) ? \
							new_client_fd : server->highest_fd;
	server->clients[new_client_fd].address = address;
	server->clients[new_client_fd].id = server->client_id++;
	FD_SET(new_client_fd, &server->monitored_set);
	sprintf(server->send_buffer,											\
			"server: client %d just arrived\n",								\
			server->clients[new_client_fd].id);
	broadcast(new_client_fd);
	return (0);
}

// -------------------------------------------------------------------------- //

void	miniServer(void)
{
	t_server	*server;
	int			fd_iterator;

	server = NULL;
	getServer(&server);

	while (true)
	{
		server->read_set	= server->monitored_set;
		server->write_set	= server->monitored_set;
		if (select(server->highest_fd + 1,									\
					&server->read_set,										\
					&server->write_set,										\
					NULL,													\
					NULL) < 0)
					continue;
		else
		{
			fd_iterator = server->fd;
			while (fd_iterator <= server->highest_fd)
			{
				if (FD_ISSET(fd_iterator, &server->read_set))
				{
					if (fd_iterator == server->fd)
						{
							if (acceptClient() < 0)
								continue ;
						}
					else
						receiveData(fd_iterator);
					break;
				}
				fd_iterator++;
			}
		}
	}
	return ;
}

// -------------------------------------------------------------------------- //

void	createSocket(void)
{
	t_server	*server;

	server = NULL;
	getServer(&server);

	server->address.sin_family		= AF_INET;
	server->address.sin_port		= htons(server->port);
	server->address.sin_addr.s_addr	= htonl(INADDR_ANY);

	server->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->fd < 0 ||													\
		bind(server->fd,													\
			(const struct sockaddr *)&server->address,						\
			sizeof(server->address)) < 0 ||									\
		listen(server->fd, SOMAXCONN) < 0)
		error("Fatal error", STDERR_FILENO, EXIT_FAILURE);
	FD_SET(server->fd, &server->monitored_set);
	server->highest_fd = server->fd;
	return ;
}

// -------------------------------------------------------------------------- //

int	main(int argc, char **argv)
{
	if (argc != 2)
		error("Wrong number of arguments", STDERR_FILENO, EXIT_FAILURE);
	convertPort(argv[1]);
	createSocket();
	miniServer();
	return (EXIT_FAILURE);
}
