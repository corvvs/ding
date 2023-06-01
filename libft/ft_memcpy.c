/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 19:09:05 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/19 23:47:12 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

#ifdef GO_FAST

static void	*mini_memcpy(
	unsigned char *s1, const unsigned char *s2, size_t n1, size_t n2
)
{
	while (n1-- && n2--)
	{
		*s1++ = *s2++;
	}
	return ((unsigned char *)s1);
}

static void	*fast_memcpy(
	unsigned char *s1, const unsigned char *s2, size_t n
)
{
	unsigned char				*u1;
	const unsigned char			*u2;
	unsigned long long			*ull1;
	const unsigned long long	*ull2;

	u1 = mini_memcpy(s1, s2, ft_memrest(s1), n);
	u2 = s2;
	u2 += ((uintptr_t)u1 - (uintptr_t)s1);
	n -= ((uintptr_t)u1 - (uintptr_t)s1);
	ull1 = (unsigned long long *)u1;
	ull2 = (const unsigned long long *)u2;
	while (n & ~(sizeof(unsigned long long) - 1))
	{
		*ull1++ = *ull2++;
		n -= sizeof(unsigned long long);
	}
	u1 = (unsigned char *)ull1;
	u2 = (unsigned char *)ull2;
	mini_memcpy(u1, u2, n, n);
	return (s1);
}

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char		*ud;
	const unsigned char	*us;

	if (dst == src || n == 0)
	{
		return (dst);
	}
	if ((n & (~(32ull - 1))) && ft_memrest(dst) == ft_memrest(src))
	{
		return (fast_memcpy(dst, src, n));
	}
	ud = dst;
	us = src;
	while (n--)
	{
		*ud++ = *us++;
	}
	return (dst);
}

#else

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char		*ud;
	const unsigned char	*us;

	if (dst == src || n == 0)
	{
		return (dst);
	}
	ud = dst;
	us = src;
	while (n--)
	{
		*ud++ = *us++;
	}
	return (dst);
}

#endif
