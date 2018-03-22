/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_stat.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ygarrot <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/03/19 09:48:05 by ygarrot           #+#    #+#             */
/*   Updated: 2018/03/22 12:03:55 by ygarrot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ls.h"

char	if_forest(mode_t st_mode)
{
	if ((st_mode & S_IFMT) == S_IFBLK)
		return ('b');
	else if ((st_mode & S_IFMT) == S_IFCHR)
		return ('c');
	else if ((st_mode & S_IFMT) == S_IFDIR)
		return ('d');
	else if ((st_mode & S_IFMT) == S_IFIFO)
		return ('p');
	else if ((st_mode & S_IFMT) == S_IFLNK)
		return ('l');
	else if ((st_mode & S_IFMT) == S_IFSOCK)
		return ('s');
	else if ((st_mode & S_IFMT) == S_IFREG)
		return ('-');
	return ('-');
}

char	special_bit(mode_t st_mode, int decal, int row)
{
	if (row == 0 && (st_mode & S_ISVTX))
		return (st_mode & decal ? 't' : 'T');
	else if (row == 1 && st_mode & S_ISGID)
		return (st_mode & decal ? 's' : 'S');
	else if (row == 2 && st_mode & S_ISUID)
		return (st_mode & decal ? 's' : 'S');
	return ('\0');
}

void	convert_mode(mode_t st_mode, t_ls *list)
{
	char	s[12];
	char attr[256];
	int		i;
	char	ac;

	i = -1;
	ft_bzero(attr, sizeof(attr));
	while (++i < 3)
	{
		s[10 - 3 - (i * 3)] = (st_mode & (4 << (i * 3))) ? 'r' : '-';
		s[10 - 2 - (i * 3)] = (st_mode & (2 << (i * 3))) ? 'w' : '-';
		if ((ac = special_bit(st_mode, st_mode & (1 << (i * 3)), i)))
			s[10 - 1 - (i * 3)] = ac;
		else
			s[10 - 1 - (i * 3)] = (st_mode & (1 << (i * 3))) ? 'x' : '-';
	}
	s[0] = if_forest(st_mode);
	ac = (listxattr(list->path, attr, 255, 0) > 0 ? '@' : ' ');
	ac = acl_get_link_np(list->path, ACL_TYPE_EXTENDED) ? '+' : ac;
	s[10] = ac;
	s[11] = '\0';
	ft_printf("%s ", s);
}

t_ls	*path_is_valid(t_global *g, char *path, struct stat *stat)
{
	struct stat st;
	t_ls	*tmp;
	(void)g;
	if (-1 == lstat(path, stat))
	{
		ls_error(path);
		return (NULL);
	}
	mallcheck(tmp = (t_ls*)ft_memalloc(sizeof(t_ls)));
	tmp->stat = st;
	tmp->path = path;
	get_stat(g, tmp);
	return (tmp);
}
void		get_stat(t_global *g, t_ls *list)
{
	//if (list->name && !g->b)
	//	return ;
	if (!g->b)
	{
		list->type = g->dir->d_type;
		list->name = ft_strdup(g->dir->d_name);
	}
	g->l_len = ft_ismax(g->l_len, ft_row_div(list->stat.st_nlink, 10));
	if (list->type != DT_CHR)
		g->s_len = ft_ismax(g->s_len, ft_row_div(list->stat.st_size, 10));
	else
	{
		list->min = minor(list->stat.st_rdev);
		list->maj = major(list->stat.st_rdev);
		g->min_len = ft_ismax(g->min_len, 1 + ft_row_div(list->min, 10));
		g->maj_len = ft_ismax(g->maj_len, 1 + ft_row_div(list->maj, 10));
	}
	list->uid = getpwuid(list->stat.st_uid)->pw_name;
	list->gid = getgrgid(list->stat.st_gid)->gr_name;
	g->u_len = ft_ismax(g->u_len, 1  + ft_strlen(list->uid));
	g->g_len = ft_ismax(g->g_len, 1 + ft_strlen(list->gid));
	g->nb_block += list->stat.st_blocks;
}


void	print_stat(t_global *g, t_ls *list)
{
	char	**tab;

	//ft_printf("{red}%d, %d{reset}\n", g->s_len, g->g_len);
	convert_mode(list->stat.st_mode, list);
	ft_printf("%*u ", g->l_len, (unsigned long)list->stat.st_nlink);
	//printf("Propriétaires :                    UID=%ld   GID=%ld\n",
	//		(long) list->stat.st_uid, (long) list->stat.st_gid);
	ft_printf("%-*s %-*s ", g->u_len, list->uid, g->g_len, list->gid);
	list->type != DT_CHR ? ft_printf("%*u ", g->s_len, (unsigned long)list->stat.st_size)
		: ft_printf("%*d, %*d ", g->maj_len,list->maj,g->min_len, list->min);
	//printf("Dernier changement d’état :        %s", ctime(&list->stat.st_ctime));
	//printf("Dernier accès au fichier :         %s", ctime(&list->stat.st_atime));
	tab = ft_strsplit(ctime(&list->stat.st_mtime), ' ');
	ft_printf("%s %2s %.5s ", tab[1], tab[2], tab[3]);
	ft_free_dblechar_tab(tab);
}
