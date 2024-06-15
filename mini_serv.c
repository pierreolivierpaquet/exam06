/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 21:13:12 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/14 20:11:47 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in -> ubuntu
#include <stdbool.h>

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
	struct sockaddr_in server_socket;

	port = 0;
	if (argc != 2 || argv == NULL)
		return (ft_putstring_fd("Wrong number of arguments", STDERR_FILENO), EXIT_FAILURE);
	convert_port(&port, argv[1]);
	if (port < 1024 || port > 65535)
		return (EXIT_FAILURE);
	
	
	
	return (EXIT_SUCCESS);
}

// [X] Check validity of arguments.
// [] Create a socket (sockaddr_in) for the server itself.
// [] Bind the socket to network.
// [] listen for incoming connetions.
// [] accept incoming connections. 