/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 20:23:59 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/19 20:37:53 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*joined;
	size_t	n1;
	size_t	n2;

	if (s1 == NULL || s2 == NULL)
	{
		return (NULL);
	}
	n1 = ft_strlen(s1);
	n2 = ft_strlen(s2);
	joined = malloc(sizeof(char) * (n1 + n2 + 1));
	if (joined != NULL)
	{
		ft_memcpy(joined, s1, n1);
		ft_memcpy(joined + n1, s2, n2 + 1);
	}
	return (joined);
}
