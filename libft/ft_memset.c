/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 19:01:36 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/19 23:33:45 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

static void	*mini_memset(unsigned char *dst, unsigned char ch,
	size_t n1, size_t n2)
{
	size_t	n;

	n = n1;
	if (n2 < n)
		n = n2;
	while (n--)
	{
		*dst++ = ch;
	}
	return (dst);
}

#ifdef GO_FAST

void	*ft_memset(void *dst, int ch, size_t n)
{
	unsigned char		*ud;
	const unsigned char	c = ch;
	unsigned long long	*ulld;
	unsigned long long	ullc;

	ud = mini_memset(dst, ch, ft_memrest(dst), n);
	n -= ((uintptr_t)ud - (uintptr_t)dst);
	ullc = c;
	ullc |= ullc << 8;
	ullc |= ullc << 16;
	ullc |= ullc << 32;
	ulld = (unsigned long long *)ud;
	while (n & ~(sizeof(unsigned long long) - 1))
	{
		*ulld++ = ullc;
		n -= sizeof(unsigned long long);
	}
	mini_memset((unsigned char *)ulld, ch, n, n);
	return (dst);
}

#else

void	*ft_memset(void *dst, int ch, size_t n)
{
	mini_memset(dst, ch, n, n);
	return (dst);
}

#endif
