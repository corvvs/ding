/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 22:57:24 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/17 01:27:46 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

static size_t	n_length(int n)
{
	size_t	i;

	i = (n <= 0);
	while (n != 0)
	{
		n /= 10;
		i += 1;
	}
	return (i);
}

char	*ft_itoa(int n)
{
	char	*str;
	size_t	len;
	int		was_negative;

	was_negative = (n < 0);
	len = n_length(n);
	str = malloc(sizeof(char) * (len + 1));
	if (str == NULL)
	{
		return (NULL);
	}
	str[len] = '\0';
	while (len--)
	{
		str[len] = "9876543210123456789"[n % 10 + 9];
		n /= 10;
	}
	if (was_negative)
	{
		str[0] = '-';
	}
	return (str);
}
