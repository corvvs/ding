/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 19:22:28 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/19 20:15:23 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

size_t	ft_strlcat(char *dst, const char *src, size_t n)
{
	size_t	dst_size;

	if (n == 0)
	{
		return (ft_strlen(src));
	}
	dst_size = ft_strnlen(dst, n);
	if (dst_size == n)
	{
		return (ft_strlen(src) + n);
	}
	return (ft_strlcpy(dst + dst_size, src, n - dst_size) + dst_size);
}
