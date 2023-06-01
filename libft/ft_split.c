/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: corvvs <corvvs@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/16 22:37:50 by corvvs            #+#    #+#             */
/*   Updated: 2023/02/27 17:10:32 by corvvs           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"
#include <stdlib.h>

static char const	*skip_delimiters(char const *s, char c)
{
	while (*s != '\0' && *s == c)
	{
		++s;
	}
	return (s);
}

static char const	*skip_word(char const *s, char c)
{
	while (*s != '\0' && *s != c)
	{
		++s;
	}
	return (s);
}

static char	**allocate_words(char const *s, char c)
{
	size_t		n;
	char const	*t;

	n = 0;
	while (*s != '\0')
	{
		s = skip_delimiters(s, c);
		t = skip_word(s, c);
		if (s == t)
			break ;
		s = t;
		++n;
	}
	return (malloc(sizeof(char *) * (n + 1)));
}

static char	**fill_words(char const *s, char c, char **words)
{
	size_t		i;
	char const	*t;

	i = 0;
	while (*s)
	{
		s = skip_delimiters(s, c);
		t = skip_word(s, c);
		if (s == t)
			break ;
		words[i] = ft_strndup(s, t - s);
		if (words[i++] == NULL)
		{
			i = 0;
			while (words[i] != NULL)
				free(words[i++]);
			free(words);
			return (NULL);
		}
		s = t;
	}
	words[i] = NULL;
	return (words);
}

char	**ft_split(char const *s, char c)
{
	char	**words;

	if (s == NULL)
	{
		return (NULL);
	}
	words = allocate_words(s, c);
	if (words == NULL)
	{
		return (NULL);
	}
	return (fill_words(s, c, words));
}
