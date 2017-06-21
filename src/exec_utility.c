/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utility.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kbagot <kbagot@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/03/18 20:58:40 by kbagot            #+#    #+#             */
/*   Updated: 2017/06/10 14:08:17 by kbagot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sh.h"

static int	invalid_exec(char **stin, char **env, t_data *data)
{
	if ((stin[0] && (access(stin[0], X_OK) == -1)) || !stin[0])
	{
		ft_tabdel(&env);
		if (stin[0])
		{
			ft_putstr_fd("minishell: command not found: ", 2);
			ft_putstr_fd(stin[0], 2);
			ft_putchar_fd('\n', 2);
			data->rvalue = 1;
		}
		return (1);
	}
	return (0);
}

static void	sig(int rvalue, pid_t pid, char *stin)
{
	if (WTERMSIG(rvalue) == SIGSEGV)
	{
		ft_putnbr_fd(pid, 2);
		ft_putchar_fd(' ', 2);
		ft_putstr_fd("segmentation fault", 2);
		ft_putstr_fd("  ", 2);
		ft_putstr_fd(stin, 2);
		ft_putchar_fd('\n', 2);
	}
	else if (WTERMSIG(rvalue) == SIGBUS)
	{
		ft_putnbr_fd(pid, 2);
		ft_putchar_fd(' ', 2);
		ft_putstr_fd("bus error", 2);
		ft_putstr_fd("  ", 2);
		ft_putstr_fd(stin, 2);
		ft_putchar_fd('\n', 2);
	}
}

static void	exec_utility(char **env, char **stin, t_data *data)
{
	pid_t	pid;
	int		rvalue;

	rvalue = 0;
	if (invalid_exec(stin, env, data))
		return ;
	if ((pid = fork()) == 0)
	{
		if (execve(stin[0], stin, env) == -1)
		{
			ft_putstr_fd("Exec format error\n", 2);
			_exit(1);
		}
	}
	else if (pid > 0)
	{
		signo = pid;
		wait(&rvalue);
		if (data->lastpid != 0)
			kill(data->lastpid, SIGTERM);
		if (WIFEXITED(rvalue))
			data->rvalue = WEXITSTATUS(rvalue);
		else if (WIFSIGNALED(rvalue))
			sig(rvalue, pid, stin[0]);
	}
	ft_tabdel(&env);
}

static void		find_pipe(char **cstin, int *j, int *k)
{
	int i;

	i = 0;
	while (cstin[i] && ft_strcmp(cstin[i], "|") != 0)
	{
		if (!ft_strchr("\"\'", cstin[i][0] && (ft_strchr(cstin[i], '>') || ft_strchr(cstin[i], '<'))))
			*k += 1;
		else
			*j += 1;
		i++;
	}
}

static void		stock_redir_fd(t_line *line, char **cstin, int *k, char *c)
{
	char *tmp;
	char *tmp2;

	tmp = ft_strsub(cstin[0], 0, c - cstin[0]);
	tmp2 = ft_strjoin(c, " ");
	line->redirect[*k] = join(ft_strjoin(tmp, " "), tmp2, cstin[1]);
	ft_strdel(&tmp);
	ft_strdel(&tmp2);
	*k += 1;
}

static void		stock_redir(char **cstin, t_line *line, int *j, int *k)
{
	char *c;

	c = NULL;
	if (!(c = ft_strchr(*cstin, '>')))
		c = ft_strchr(*cstin, '<');
	if (c == *cstin)
	{
		line->redirect[*k] = join(ft_strdup(*cstin), " ", cstin[1]);
		*k += 1;
	}
	else
	{
		if (!ft_isnbr(ft_strsub(*cstin, 0, c - *cstin)))
		{
			line->proc[*j] = ft_strsub(*cstin, 0, c - *cstin);
			line->redirect[*k] = join(ft_strdup(&cstin[0][c - *cstin]), " ",
					cstin[1]);
			*j += 1;
			*k += 1;
		}
		else
			stock_redir_fd(line, cstin, k, c);
	}
}

static void		separate_line(char **cstin, int *i, t_line *line)
{
	int j;
	int k;

	k = 0;
	j = 0;
	while (cstin[*i] && (ft_strcmp(cstin[*i], "|") != 0))
	{//add redirect cmd
		char *c;
		if (!ft_strchr("\'\"", cstin[*i][0]) && ((c = ft_strchr(cstin[*i], '>')) || (c = ft_strchr(cstin[*i], '<'))))
		{
			stock_redir(&cstin[*i], line, &j, &k);
			*i += 1;
		}
		else
		{
			if (ft_strchr("\'\"", cstin[*i][0]))
				line->proc[j++] = ft_strdup(&cstin[*i][1]);
			else
				line->proc[j++] = ft_strdup(cstin[*i]);
		}
		if (!cstin[*i])
			break ;
		*i += 1;
	}
	line->redirect[k] = NULL;
	line->proc[j] = NULL;
}

static t_line	*split_pipe(char **cstin)
{
	t_line *line;
	int		i;
	int		j;
	int		k;
	t_line *save;

	save = NULL;
	i = 0;
	j = 0;
	k = 0;
	while (cstin[i])
	{
		if (save == NULL)
		{
			line = ft_memalloc(sizeof(t_line));
			save = line;
		}
		find_pipe(&cstin[i], &j, &k);
		line->proc = (char**)malloc(sizeof(char*) * j + 1);
		line->redirect = (char**)malloc(sizeof(char*) * k + 1);
		separate_line(cstin, &i, line);
		line->next = NULL;
		if (!cstin[i])
			break ;
		line->next = ft_memalloc(sizeof(t_line));
		line = line->next;
		i++;
	}
	return (save);
}

static t_line	*fork_pipes(t_line *line, t_data *d)
{
	t_line *old;
	pid_t	pid;
	int		fd[2];

	pid = 0;
	if (line)
	{
		while (line->next)
		{
			pipe(fd);
			d->out = fd[1];
			if ((pid = fork ()) == 0)
			{
				if (d->in != 0)
				{
					dup2 (d->in, 0);
					close (d->in);
				}
				if (d->out != 1)
				{
					dup2 (d->out, 1);
					close (d->out);
				}
				return (line);
			}
			d->lastpid = pid;
			close (d->out);
			d->in = fd[0];
			old = line;
			line = line->next;
			ft_tabdel(&old->proc);
			ft_tabdel(&old->redirect);
			free(old);
		}
		if (d->in != 0)
			dup2(d->in, 0);
	}
	return (line);
}


static void	redir_output(char **cmd, t_data *d, int i)
{//  >
	int up;

	if (i != 1)
		up = ft_atoi(cmd[0]);
	else
		up = 1;
	if ((d->out = open(cmd[i], O_CREAT | O_WRONLY | O_TRUNC,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) != -1)
		dup2(d->out, up);
	else
		ft_putstr_fd("21sh: permission denied\n", 2);
	if (!(d->out >= 0 && d->out <= 2))
		close (d->out);
}

static void	redir_input(char **cmd, t_data *d, int i)
{// <
	int up;

	if (i != 1)
		up = ft_atoi(cmd[0]);
	else
		up = 0;
	if ((d->in = open(cmd[i], O_RDONLY)) != -1)
		dup2(d->in, up);
	else
		ft_putstr_fd("21sh: permission denied\n", 2);
	if (!(d->in >= 0 && d->in <= 2))
		close (d->in);
}

static void	append_redir_output(char **cmd, t_data *d, int i)
{// >>
	int up;

	if (i != 1)
		up = ft_atoi(cmd[0]);
	else
		up = 1;
	if ((d->out = open(cmd[i], O_CREAT | O_WRONLY | O_APPEND,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) != -1)
		dup2(d->out, up);
	else
		ft_putstr_fd("21sh: permission denied\n", 2);
	if (!(d->in >= 0 && d->in <= 2))
		close (d->out);
}

static void	dup_output(char **cmd, t_data *d, int i)
{
	int up;

	if (i != 1)
		up = ft_atoi(cmd[0]);
	else
		up = 1;
	if (!(ft_isnbr(cmd[i])) && ft_strcmp(cmd[i], "-"))
		d->out = open(cmd[i], O_CREAT | O_WRONLY | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	else if (!ft_strcmp(cmd[i], "-"))
		close (up);
	else
		d->out = ft_atoi(cmd[i]);
		d->out = dup2(d->out, up);
		if (!(d->out >= 0 && d->out <= 2))
			close (d->out);
}

static void	dup_input(char **cmd, t_data *d, int i)
{
	int up;

	if (i != 1)
		up = ft_atoi(cmd[0]);
	else
		up = 0;
	if (!(ft_isnbr(cmd[i])) && ft_strcmp(cmd[i], "-"))
		d->out = open(cmd[i], O_CREAT | O_WRONLY | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	else if (!ft_strcmp(cmd[i], "-"))
		close (up);
	else
		d->out = ft_atoi(cmd[i]);
	d->out = dup2(d->out, up);
	if (!(d->out >= 0 && d->out <= 2))
		close (d->out);
}

static void exec_redir(char **rdr, t_data *d)
{
	int i;
	int j;
	char **cmd;

	j = 0;
	i = 0;
	cmd = NULL;
	while (rdr[j])
	{
		cmd = ft_strsplit(rdr[j], ' ');
		while (cmd[i])
			i++;
		i--;
		if (i >= 0 && !ft_strcmp(cmd[i - 1], ">"))
			redir_output(cmd, d, i);
		else if (i >= 0 && !ft_strcmp(cmd[i - 1], "<"))
			redir_input(cmd, d, i);
		else if (i >= 0 && !ft_strcmp(cmd[i - 1], ">>"))
			append_redir_output(cmd, d, i);
		else if (i >= 0 && !ft_strcmp(cmd[i - 1], ">&"))
			dup_output(cmd, d, i);
		else if (i >= 0 && !ft_strcmp(cmd[i - 1], "<&"))
			dup_input(cmd, d, i);
		i = 0;
		j++;
	}
	ft_tabdel(&cmd);
}

int		get_proc(int sign)
{
	static int val = 2;

	if (sign != 0)
		val = sign;
	return (val);
}


void kill_procs(int sig)
{
	if (sig == SIGINT)
	{
		if (signo > 0)
		{
			kill(signo, 9);
			ft_putchar('\n');
		}
	}
}

static void reset_fd(t_data *data)
{
	dup2(data->stdin_cpy, 0);
	close(data->stdin_cpy);
	dup2(data->stdout_cpy, 1);
	close(data->stdout_cpy);
	dup2(data->stderr_cpy, 2);
	close(data->stderr_cpy);
	if (data->in != 0)
		close (data->in);
	if (data->out != 1)
		close (data->out);
	if (data->err != 2)
		close (data->err);
}

void		parse_entry(t_env **s_env, char **cstin, char *stin, t_data *data)
{
	t_env	*tmp_env;
	t_line	*line;

	data->stdin_cpy = 10;
	data->stdout_cpy = 11;
	data->stderr_cpy = 12;
	dup2(0, 10);
	dup2(1, 11);
	dup2(2, 12);
	data->in = 0;
	data->out = 1;
	data->err = 2;
	line = split_pipe(cstin);
	line = fork_pipes(line, data);
	exec_redir(line->redirect, data);
	cstin = line->proc;
	tmp_env = NULL;
	if ((data->rvalue = builtin(cstin, s_env, stin, data)))
	{
		if (line->next)
			_exit(data->rvalue);
		else
		{
			reset_fd(data);
			return ;
		}
	}
	tmp_env = master_env(*s_env, cstin, tmp_env);
	if ((cstin = utility(cstin, *s_env)))
		exec_utility(list_to_tab(tmp_env), cstin, data);
	if (line->next)
		_exit(data->rvalue);
	destroy_env(&tmp_env);
	ft_tabdel(&line->proc);
	ft_tabdel(&line->redirect);
	free(line);

	int status;
	int pid;
	while ((pid = wait(&status)) > 0)
		status = 0;;
	reset_fd(data);
} 
