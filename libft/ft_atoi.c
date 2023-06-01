/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 18:43:05 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/19 16:10:48 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>
#include <limits.h>

const unsigned char	g_atoitable[] = (const unsigned char []){
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1,
};

static int	is_of(long num, long d, int sign)
{
	if (sign > 0)
	{
		return ((num > LONG_MAX / 10)
			|| (num == LONG_MAX / 10 && d > LONG_MAX - num * 10));
	}
	else
	{
		return ((num > -(LONG_MIN / 10))
			|| (num == -(LONG_MIN / 10) && d > -(LONG_MIN + num * 10)));
	}
}

static int	ft_isspace(unsigned char ch)
{
	return (!((unsigned int)ch & ~63u) && (0x100003e00ull & (1ull << ch)));
}

static int	atoi_isdigit(unsigned char ch)
{
	return (!(128u & g_atoitable[ch]));
}

int	ft_atoi(const char *str)
{
	long	num;
	int		sign;

	while (ft_isspace(*str))
	{
		++str;
	}
	num = 0;
	sign = 1 - 2 * (*str == '-');
	str = str + (*str == '+' || *str == '-');
	while (atoi_isdigit(*str))
	{
		if (is_of(num, g_atoitable[(unsigned char)*str], sign))
		{
			if (sign > 0)
			{
				return ((int)LONG_MAX);
			}
			return ((int)LONG_MIN);
		}
		num = num * 10 + g_atoitable[(unsigned char)*str];
		++str;
	}
	return (num * sign);
}
