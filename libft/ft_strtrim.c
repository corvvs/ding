/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 20:34:22 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/27 17:10:24 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

char	*ft_strtrim(char const *s1, char const *set)
{
	size_t	len;

	if (s1 == NULL || set == NULL)
	{
		return (NULL);
	}
	while (*s1 != '\0' && ft_strchr(set, *s1) != NULL)
	{
		++s1;
	}
	len = ft_strlen(s1);
	while (len && ft_strchr(set, s1[len - 1]) != NULL)
	{
		--len;
	}
	return (ft_strndup(s1, len));
}
