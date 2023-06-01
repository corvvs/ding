/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 18:34:52 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/27 17:33:33 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

char	*ft_strnstr(const char *haystack, const char *needle, size_t len)
{
	size_t	needle_len;
	size_t	i;

	if (needle[0] == '\0')
		return ((char *)haystack);
	if (len == 0)
		return (NULL);
	len = ft_strnlen(haystack, len);
	needle_len = ft_strnlen(needle, len + 1);
	if (len < needle_len)
		return (NULL);
	i = 0;
	while (i + needle_len <= len)
	{
		if (ft_memcmp(&haystack[i], needle, needle_len) == 0)
			return ((char *)&haystack[i]);
		++i;
	}
	return (NULL);
}
