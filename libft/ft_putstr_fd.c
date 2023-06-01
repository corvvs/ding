/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putstr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/17 01:34:51 by corvvs            #+#    #+#             */
/*   Updated: 2023/03/21 03:08:29 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <unistd.h>

void	ft_putstr_fd(char *str, int fd)
{
	size_t	i;
	ssize_t	w;

	(void)w;
	if (str == NULL)
	{
		return ;
	}
	i = 0;
	while (str[i] != '\0')
	{
		++i;
		if (i == (1 << 16))
		{
			w = write(fd, str, i);
			str += i;
			i = 0;
		}
	}
	if (i > 0)
	{
		w = write(fd, str, i);
	}
}
