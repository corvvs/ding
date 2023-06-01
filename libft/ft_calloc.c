/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 19:23:01 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/16 19:26:42 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

void	*ft_calloc(size_t count, size_t size)
{
	void	*mem;

	if (count == 0 || size == 0)
	{
		count = 1;
		size = 1;
	}
	if (count != 0 && SIZE_MAX / count < size)
	{
		errno = ENOMEM;
		return (NULL);
	}
	mem = malloc(count * size);
	if (mem)
	{
		ft_bzero(mem, count * size);
	}
	return (mem);
}
