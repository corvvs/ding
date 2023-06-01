/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 19:05:54 by corvvs            #+#    #+#             */
/*   Updated: 2023/03/05 05:37:28 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>
#include <stdio.h>

static unsigned char	*mini_memcmp(
	const unsigned char *s1, const unsigned char *s2,
	size_t n1, size_t n2)
{
	size_t	n;

	n = n1;
	if (n2 < n)
		n = n2;
	while (n--)
	{
		if (*s1 ^ *s2)
		{
			break ;
		}
		++s1;
		++s2;
	}
	return ((unsigned char *)s1);
}

#ifdef GO_FAST

static int	ft_memcmp_fast(
	const unsigned char *s1, const unsigned char *s2,
	size_t n)
{
	const unsigned char			*u1;
	const unsigned long long	*ull1;
	const unsigned long long	*ull2;

	u1 = mini_memcmp(s1, s2, ft_memrest(s1), n);
	s2 += ((uintptr_t)u1 - (uintptr_t)s1);
	n -= (uintptr_t)u1 - (uintptr_t)s1;
	s1 = (unsigned char *)u1;
	ull1 = (unsigned long long *)s1;
	ull2 = (unsigned long long *)s2;
	while (n & ~(sizeof(unsigned long long) - 1))
	{
		if (*ull1 ^ *ull2)
			break ;
		n -= sizeof(unsigned long long);
		++ull1;
		++ull2;
	}
	s1 = (unsigned char *)ull1;
	s2 = (unsigned char *)ull2;
	u1 = mini_memcmp(s1, s2, n, n);
	s2 += ((uintptr_t)u1 - (uintptr_t)s1);
	if (n == (uintptr_t)u1 - (uintptr_t)s1)
		return (0);
	return (*u1 - *s2);
}

int	ft_memcmp(const void *s1, const void *s2, size_t n)
{
	int					diff;
	const unsigned char	*u1;
	const unsigned char	*u2;

	if ((n & (~(32ull - 1))) && ft_memrest(s1) == ft_memrest(s2))
	{
		return (ft_memcmp_fast(s1, s2, n));
	}
	u1 = s1;
	u2 = s2;
	diff = 0;
	while (!diff && n--)
	{
		diff = *u1++ - *u2++;
	}
	return (diff);
}

#else

int	ft_memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char	*u1;
	const unsigned char	*u2;
	const unsigned char	*u = mini_memcmp(s1, s2, n, n);

	u1 = s1;
	u2 = s2;
	if (u < u1 + n)
	{
		u2 += u - u1;
		return (*(const unsigned char *)u - *(const unsigned char *)u2);
	}
	return (0);
}

#endif
