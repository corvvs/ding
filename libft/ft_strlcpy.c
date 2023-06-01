/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcpy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 19:19:56 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/20 01:19:30 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

size_t	ft_strlcpy(char	*dst, const char *src, size_t n)
{
	const size_t	src_len = ft_strlen(src);
	size_t			copy_size;

	if (n == 0)
	{
		return (src_len);
	}
	copy_size = src_len;
	if (n - 1 < copy_size)
	{
		copy_size = n - 1;
	}
	ft_memcpy(dst, src, copy_size);
	dst[copy_size] = '\0';
	return (src_len);
}
