/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbagot <kbagot@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/03/24 17:31:43 by kbagot            #+#    #+#             */
/*   Updated: 2017/06/08 18:43:47 by kbagot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sh.h"

int			exec_echo(char **stin)
{
	int i;

	i = 1;
	while (stin[i])
	{
		ft_printf("%s", stin[i]);
		if (stin[i + 1])
			ft_putchar(' ');
		i++;
	}
	ft_putchar('\n');
	return(1);
}
