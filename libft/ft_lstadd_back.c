/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstadd_back.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/17 01:59:56 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/17 02:26:41 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

void	ft_lstadd_back(t_list **lst, t_list *new)
{
	if (lst == NULL)
	{
		return ;
	}
	if (*lst == NULL)
	{
		*lst = new;
	}
	else
	{
		ft_lstlast(*lst)->next = new;
	}
}
