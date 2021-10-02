/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adubeau <marvin@42quebec.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/30 16:53:26 by adubeau           #+#    #+#             */
/*   Updated: 2021/10/02 03:00:11 by adubeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


size_t ft_strlen(const char *str)
{
	int i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

size_t	ft_strlcpy(char *dest, const char *src, size_t n)
{
	unsigned int	i;

	i = 0;
	if (!dest || !src)
		return (0);
	if (n > 0)
	{
		while (--n && src[i])
		{
			dest[i] = src[i];
			i++;
		}
		dest[i] = '\0';
	}
	while (src[i])
		i++;
	return (i);
}

static char	**ft_malloc_error(char **tab)
{
	unsigned int	i;

	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
	return (NULL);
}

static unsigned int	ft_get_nb_strs(char const *s, char c)
{
	unsigned int	i;
	unsigned int	nb_strs;

	if (!s[0])
		return (0);
	i = 0;
	nb_strs = 0;
	while (s[i] && s[i] == c)
		i++;
	while (s[i])
	{
		if (s[i] == c)
		{
			nb_strs++;
			while (s[i] && s[i] == c)
				i++;
			continue ;
		}
		i++;
	}
	if (s[i - 1] != c)
		nb_strs++;
	return (nb_strs);
}

static void	ft_get_next_str(char **next_str, unsigned int *next_str_len,
					char c)
{
	unsigned int	i;

	*next_str += *next_str_len;
	*next_str_len = 0;
	i = 0;
	while (**next_str && **next_str == c)
		(*next_str)++;
	while ((*next_str)[i])
	{
		if ((*next_str)[i] == c)
			return ;
		(*next_str_len)++;
		i++;
	}
}

char	**ft_split(char const *s, char c)
{
	char			**tab;
	char			*next_str;
	unsigned int	next_str_len;
	unsigned int	nb_strs;
	unsigned int	i;

	nb_strs = ft_get_nb_strs(s, c);
	tab = (char **)malloc(sizeof(char *) * (nb_strs + 1));
	if (!tab || !s)
		return (NULL);
	i = 0;
	next_str = (char *)s;
	next_str_len = 0;
	while (i < nb_strs)
	{
		ft_get_next_str(&next_str, &next_str_len, c);
		tab[i] = (char *)malloc(sizeof(char) * (next_str_len + 1));
		if (!tab[i])
			return (ft_malloc_error(tab));
		ft_strlcpy(tab[i], next_str, next_str_len + 1);
		i++;
	}
	tab[i] = NULL;
	return (tab);
}

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*str;
	int		i;
	int		j;

	i = 0;
	j = 0;
	str = (char *)malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 1));
	if (!str)
		return (NULL);
	while (s1[i])
	{
		str[i] = s1[i];
		i++;
	}
	while (s2[j])
	{
		str[i + j] = s2[j];
		j++;
	}
	str[i + j] = '\0';
	return (str);
}

void	ft_putchar(char c)
{
	write(1, &c, 1);
}

void	ft_putstr(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		ft_putchar(str[i++]);
	write(1,"\n", 1);
}

char	*ft_getPath(char **envp)
{
	int i;

	while (envp++)
	{
		if (envp[i][0] == 'P')
			if (envp[i][1] == 'A')
				return (*envp);
	}
	return (*envp);
}

char **ft_getCmd(char **argv, int n)
{
	return (ft_split(argv[n], ' '));
}

void	ft_exec(char **argv, char **envp,  int n)
{
	char **command;
	char **path;
	char *cmd;
	int i;

	i = -1;
	path = ft_split(ft_getPath(envp), ':');
	command = ft_getCmd(argv, n);
	while (path[++i])
	{
		cmd = ft_strjoin(ft_strjoin(path[i], "/"), command[0]);
		execve(cmd, (char *const *) command, envp);
		free(cmd);
	}
	ft_putstr("Cette commande n'existe pas");
	exit(1);
}

void	ft_child(int *end, char **envp, char **argv)
{
	int f1;


	if ((f1 = open(argv[1], O_RDONLY)) < 0)
	{
		ft_putstr("fichier1 manquant");
		exit (1);
	}
	close(end[0]);
	dup2(end[1], 1);
	close(end[1]);
	dup2(f1, 0);
	ft_exec(argv, envp, 2);

}

void	ft_parent(int *end, char **envp, char **argv, pid_t *child)
{
	int status;
	int f2;

	status = 0;
	waitpid(*child, &status, WNOHANG);
	if ((f2 = open(argv[4], O_RDWR | O_TRUNC, 0644)) < 0)
	{
		ft_putstr("fichier2 manquant");
		exit(1);
	}
	close(end[1]);
	dup2(end[0], 0);
	close(end[0]);
	dup2(f2, 1);
	ft_exec(argv, envp, 3);
}

void	pipex(int *end, char **envp, char **argv)
{
	pid_t	child;

	if ((child = fork()) < 0)
		return (perror("Bad fork"));
	else if (child == 0)
		ft_child(end, envp, argv);
	else if (child > 0)
		ft_parent(end, envp, argv, &child);

}

int main(int argc, char **argv, char **envp)
{
	int	end[2];

	if (argc != 5)
	{
		ft_putstr("Entrez la commande sous la forme: \"./pipex fichier1 cmd1 cmd2 fichier2\""); 
		exit(1);
	}
	if (pipe(end) == -1)
	{
		ft_putstr("Bad pipe");
		exit(1);
	}
	pipex(end, envp, argv);
	return (0);
}
