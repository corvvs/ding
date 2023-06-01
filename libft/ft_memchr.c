/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 19:07:06 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/19 23:43:14 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef GO_FAST

# define HIMAGIC 0x8080808080808080ull
# define LOMAGIC 0x0101010101010101ull

static void	*mini_memchr(const unsigned char *mem, unsigned char ch,
	size_t n1, size_t n2)
{
	size_t	n;

	n = n1;
	if (n2 < n)
		n = n2;
	while (n-- && *mem != ch)
	{
		++mem;
	}
	return ((void *)mem);
}

static void	*fast_memchr(const unsigned char *mem, unsigned char ch, size_t n)
{
	const unsigned char			*u;
	const unsigned long long	*ull;
	unsigned long long			stamp;
	unsigned long long			w;
	const size_t				n0 = n;

	u = mini_memchr(mem, ch, ft_memrest(mem), n);
	n -= (uintptr_t)u - (uintptr_t)mem;
	ull = (const unsigned long long *)u;
	stamp = ch;
	stamp |= stamp << 8;
	stamp |= stamp << 16;
	stamp |= stamp << 32;
	while (n & ~(sizeof(unsigned long long) - 1))
	{
		w = *ull ^ stamp;
		if (((w - LOMAGIC) & ~w & HIMAGIC) != 0)
			break ;
		++ull;
		n -= sizeof(unsigned long long);
	}
	u = mini_memchr((const unsigned char *)ull, ch, n, n);
	if (n0 == (uintptr_t)u - (uintptr_t)mem)
		return (NULL);
	return ((void *)u);
}

void	*ft_memchr(const void *mem, int ch, size_t n)
{
	const unsigned char	*u;
	const unsigned char	uc = ch;

	if (n & (~(32ull - 1)))
	{
		return (fast_memchr(mem, ch, n));
	}
	u = mem;
	while (n--)
	{
		if (*u == uc)
			return ((void *)u);
		u++;
	}
	return (NULL);
}

#else

void	*ft_memchr(const void *mem, int ch, size_t n)
{
	const unsigned char	*u;
	const unsigned char	uc = ch;

	u = mem;
	while (n--)
	{
		if (*u == uc)
			return ((void *)u);
		u++;
	}
	return (NULL);
}

#endif
