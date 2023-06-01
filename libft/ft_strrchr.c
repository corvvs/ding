/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 18:21:51 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/17 01:12:53 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int ch)
{
	const char	c = ch;
	char		*r;

	r = NULL;
	while (*s != '\0')
	{
		if (*s == c)
		{
			r = (char *)s;
		}
		++s;
	}
	if (*s == c)
	{
		r = (char *)s;
	}
	return (r);
}
