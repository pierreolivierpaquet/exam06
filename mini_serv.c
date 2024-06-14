/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ppaquet <pierreolivierpaquet@hotmail.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 21:13:12 by ppaquet           #+#    #+#             */
/*   Updated: 2024/06/14 19:40:31 by ppaquet          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>

int	ft_putstring_fd(char *string, int fd)
{
	if (string == NULL || fd < 0)
		return (-1);
	write(fd, string, strlen(string));
	write(fd, "\n", 1);
	return (0);
}

int	main(int argc, char **argv)
{
	if (argc != 2 || argv == NULL)
		return (ft_putstring_fd("Wrong number of arguments", STDERR_FILENO), EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

// [] Check validity of arguments.
// [] Create a socket for the server itself.
// [] Bind the socket to network.
// [] listen for incoming connetions.
// [] accept incoming connections. 