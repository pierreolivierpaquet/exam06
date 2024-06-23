/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/23 13:29:59 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/23 16:37:12 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>			// sprintf()
#include <stdlib.h>			// malloc()
#include <unistd.h>			// write()
#include <sys/socket.h>		// socket()
#include <sys/select.h>		// ubuntu -> on macos, included in unistd.h
#include <string.h>			// strlen()
#include <netinet/in.h>		// ubuntu -> sockaddr_in
#include <stdbool.h>

typedef struct	s_client
{
	int					id;
	char				buffer[1024];
	struct	sockaddr_in	client_address;
}				t_client;

typedef struct	s_server
{
	int					fd;
	int					port;
	struct sockaddr_in	server_address;
	int					highest_fd;
	t_client			clients[SOMAXCONN];
	char				receive_buffer[BUFSIZ];
	char				send_buffer[BUFSIZ];
	fd_set				monitored_set;
	fd_set				read_set;
	fd_set				write_set;
	int					client_id;
}				t_server;

// -------------------------------------------------------------------------- //

int	ft_putstring_fd(char *string, int fd)
{
	if (string == NULL || fd < 0)
	{
		return (-1);
	}
	write(fd, string, strlen(string));
	write(fd, "\n", 1);
	return (0);
}

// -------------------------------------------------------------------------- //

t_server	*getServer(t_server **get)
{
	static t_server	server;

	if (get != NULL)
	{
		*get = &server;
	}
	return (&server);
}

// -------------------------------------------------------------------------- //

int	convertPort(char *port, int *result)
{
	int	converted_port;

	if (port == NULL || result == NULL)
		return (-1);

	converted_port = 0;
	converted_port = atoi(port);
	if (converted_port < 1024 || converted_port > 65535)
		converted_port = -1;
	*result = converted_port;
	return (converted_port);
}

// -------------------------------------------------------------------------- //

int	createSocket(void)
{
	t_server	*server;

	server = NULL;
	getServer(&server);

	server->server_address.sin_family = AF_INET;
	server->server_address.sin_port = htons(server->port);
	server->server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	server->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->fd < 0 ||										\
		bind(server->fd,										\
		(const struct sockaddr *)&server->server_address,		\
		sizeof(server->server_address)) ||						\
		listen(server->fd, SOMAXCONN) < 0)
	{
		ft_putstring_fd("Fatal error", STDERR_FILENO);
		return (-1);
	}
	server->highest_fd = server->fd;
	FD_SET(server->fd, &server->monitored_set);

	return (0);
}

// -------------------------------------------------------------------------- //

int	broadcast(int sender)
{
	t_server	*server;
	int			i_fd;

	getServer(&server);
	i_fd = server->fd;
	while (i_fd <= server->highest_fd)
	{
		if (FD_ISSET(i_fd, &server->write_set) && i_fd != sender)
		{
			send(i_fd, server->send_buffer, strlen(server->send_buffer), 0);
		}
		i_fd++;
	}
	bzero(server->send_buffer, strlen(server->send_buffer));
	return (0);
}

// -------------------------------------------------------------------------- //

int	acceptClient(void)
{
	t_server			*server;
	struct sockaddr_in	client_address;
	int					new_client_fd;
	socklen_t			sockaddr_lenght;

	new_client_fd = 0;
	bzero(&client_address, sizeof(client_address));
	sockaddr_lenght = sizeof(client_address);
	getServer(&server);
	new_client_fd = accept(server->fd,								\
							(struct sockaddr *)&client_address,		\
							&sockaddr_lenght);
	if (new_client_fd < 0)
		return (-1);

	if (new_client_fd > server->highest_fd)
		server->highest_fd = new_client_fd;

	server->clients[new_client_fd].id = server->client_id;
	server->client_id++;
	server->clients[new_client_fd].client_address = client_address;

	FD_SET(new_client_fd, &server->monitored_set);

	sprintf(server->send_buffer, "server: client %d just arrived\n", server->clients[new_client_fd].id);
	broadcast(new_client_fd);

	return (0);
}

// -------------------------------------------------------------------------- //

int	removeClient(int client_fd)
{
	t_server	*server;

	getServer(&server);
	sprintf(server->send_buffer,				\
			"server: client %d just left\n",	\
			server->clients[client_fd].id);
	broadcast(client_fd);
	FD_CLR(client_fd, &server->monitored_set);
	close(client_fd);
	bzero(&server->clients[client_fd], sizeof(*server->clients));
	return (0);
}

// -------------------------------------------------------------------------- //

int	handleData(int client_fd, int bytes_received)
{
	t_server	*server;
	int			i;
	int			j;

	getServer(&server);
	i = 0;
	j = strlen(server->clients[client_fd].buffer);
	while (i < bytes_received)
	{
		server->clients[client_fd].buffer[j] = server->receive_buffer[i];
		if (server->clients[client_fd].buffer[j] == '\n')
		{
			server->clients[client_fd].buffer[j] = '\0';
			sprintf(server->send_buffer, "client %d: %s\n",		\
					server->clients[client_fd].id,				\
					server->clients[client_fd].buffer);
			broadcast(client_fd);
			bzero(server->clients[client_fd].buffer, strlen(server->clients[client_fd].buffer));
			j = -1;
		}
		i++;
		j++;
	}
	return (0);
}

// -------------------------------------------------------------------------- //

int	receiveData(int	client_fd)
{
	t_server	*server;
	int	bytes_received;

	getServer(&server);
	bytes_received = 0;
	bytes_received = recv(client_fd,						\
							server->receive_buffer,			\
							sizeof(server->receive_buffer),	\
							0);
	if (bytes_received <= 0)
	{
		removeClient(client_fd);
	}
	else
	{
		handleData(client_fd, bytes_received);
	}
	return (0);
}

// -------------------------------------------------------------------------- //

void	miniServer(void)
{
	t_server	*server;
	int			i_fd;

	getServer(&server);

	while (true)
	{
		server->read_set = server->monitored_set;
		server->write_set = server->monitored_set;

		if (select(server->highest_fd + 1,		\
					&server->read_set,			\
					&server->write_set,			\
					NULL,						\
					NULL) < 0)
			continue;

		i_fd = server->fd;
		while (i_fd <= server->highest_fd)
		{
			if (FD_ISSET(i_fd, &server->read_set))
			{
				if (i_fd == server->fd)
				{
					if (acceptClient() < 0)
						continue ;
				}
				else
				{
					receiveData(i_fd);
				}
				break;
			}
			i_fd++;
		}
	}
	return ;
}

// -------------------------------------------------------------------------- //

int	main(int argc, char **argv)
{
	t_server	*server;

	if (argc != 2)
		return (ft_putstring_fd("Wrong number of arguments", STDERR_FILENO),\
				EXIT_FAILURE);
	server = NULL;
	getServer(&server);

	if (convertPort(argv[1], &server->port) < 0)
		return (ft_putstring_fd("Fatal error", STDERR_FILENO), \
				EXIT_FAILURE);

	if (createSocket() < 0)
		return (EXIT_FAILURE);

	miniServer();

	return(EXIT_SUCCESS);
}
