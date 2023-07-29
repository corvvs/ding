/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_starts_with.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 18:26:43 by corvvs            #+#    #+#             */
/*   Updated: 2023/07/29 18:56:06 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

// str の先頭が prefix で始まるなら prefix の長さを, そうでなければ -1 を返す
int	ft_starts_with(const char *str, const char *prefix)
{
	size_t	m;
	size_t	l;

	m = ft_strlen(str);
	l = ft_strlen(prefix);
	if (m < l)
		return (-1);
	if (ft_strncmp(str, prefix, l) == 0)
		return (l);
	return (-1);
}
