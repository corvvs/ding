/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 18:26:43 by corvvs            #+#    #+#             */
/*   Updated: 2023/03/27 04:10:02 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

int	ft_strcmp(const char *s1, const char *s2) {
	while (*s1 && *s1 == *s2) {
		++s1; ++s2;
	}
	return (unsigned char)*s1 - (unsigned char)*s2;
}

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	m;
	size_t	l;

	if (n == 0)
		return (0);
	m = ft_strlen(s1);
	l = ft_strlen(s2);
	if (m < l)
		m = l;
	if (m < n)
		n = m;
	return (ft_memcmp(s1, s2, n));
}
