/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 19:10:43 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/10 19:12:33 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

void	*ft_memmove(void *dst, const void *src, size_t n)
{
	unsigned char		*ud;
	const unsigned char	*us;

	if (dst <= src)
	{
		return (ft_memcpy(dst, src, n));
	}
	if (dst == src || n == 0)
	{
		return (dst);
	}
	ud = dst;
	us = src;
	while (n--)
	{
		ud[n] = us[n];
	}
	return (dst);
}
