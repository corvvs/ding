/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlen.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 18:59:26 by corvvs            #+#    #+#             */
/*   Updated: 2023/04/16 00:34:55 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

static void	*mini_strlen(const unsigned char *dst, size_t n)
{
	while (n-- && *dst)
	{
		++dst;
	}
	return ((void *)dst);
}

#if GO_FAST

# define HIMAGIC 0x8080808080808080ull
# define LOMAGIC 0x0101010101010101ull

size_t	ft_strlen(const char *str)
{
	size_t						n;
	const unsigned long long	*p;
	unsigned long long			w;

	p = mini_strlen((const unsigned char *)str, ft_memrest(str));
	n = (uintptr_t)p - (uintptr_t)str;
	while (1)
	{
		w = *p;
		if (((w - LOMAGIC) & ~w & HIMAGIC) != 0)
			break ;
		++p;
		n += sizeof(unsigned long long);
	}
	str = mini_strlen((const unsigned char *)p, sizeof(unsigned long long));
	n += (uintptr_t)str - (uintptr_t)p;
	return (n);
}

#else

size_t	ft_strlen(const char *str)
{
	const char	*s0 = str;

	while (*str)
		++str;
	return (str - s0);
}

#endif

size_t	ft_strnlen(const char *str, size_t nmax)
{
	return ((size_t)mini_strlen((const unsigned char *)str, nmax)
		- (size_t)str);
}
