/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/17 01:28:30 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/19 21:18:47 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

char	*ft_strmapi(char const *s, char (*f)(unsigned int, char))
{
	char	*mapped;
	size_t	i;

	if (s == NULL || f == NULL)
	{
		return (NULL);
	}
	mapped = ft_strdup(s);
	if (mapped == NULL)
	{
		return (NULL);
	}
	i = 0;
	while (s[i] != '\0')
	{
		mapped[i] = f(i, s[i]);
		++i;
	}
	return (mapped);
}
