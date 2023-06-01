/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putendl_fd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/17 01:41:22 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/17 01:42:47 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <unistd.h>

void	ft_putendl_fd(char *str, int fd)
{
	if (str == NULL)
	{
		return ;
	}
	ft_putstr_fd(str, fd);
	ft_putstr_fd("\n", fd);
}
