/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 19:30:39 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/27 16:24:52 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*sub;

	if (s == NULL)
	{
		return (NULL);
	}
	s += ft_strnlen(s, start);
	len = ft_strnlen(s, len);
	sub = malloc(sizeof(char) * (len + 1));
	if (sub != NULL)
	{
		ft_memcpy(sub, s, len);
		sub[len] = '\0';
	}
	return (sub);
}
