extern void	mark_pairs(int *, int *, int *, int),
		identify_pairs(),
		set_retained(int, int, int),
		init_globals();
extern int	*pre_pair(tabstruct **),
                get_index(int, int),
		get_id(int, int),
		get_pair(int, int, int),
		**select_ids(int, int *, int **, int **),
		***select_pairs(int, int *, int **, int **);
extern short int	get_fieldnr(int, int),
			get_retained(int, int);
extern set_struct *cn_pairs(int, int);

