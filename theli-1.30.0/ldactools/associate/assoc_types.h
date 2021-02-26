typedef struct {
    char	conffile[MAXCHAR];
    char	listfile[MAXCHAN][MAXCHAR];
    char	outfile[MAXCHAN][MAXCHAR];
    char	table_name[MAXCHAR];
    char	assoc_flag[MAXCHAR];
    int		ilist;
    int		mask;
    short int	shortmask;
    double	inter_tol, iso_tol, min_el_d;
    bool	equal_flag, in_pix, equal_frame, pair_cols, use_ellip;
    char	ra[MAXCHAR], dec[MAXCHAR],flag[MAXCHAR],seqnr[MAXCHAR],
                a[MAXCHAR],b[MAXCHAR],theta[MAXCHAR],e[MAXCHAR],
                cluster_nr[MAXCHAR],richness[MAXCHAR],
                fpos[MAXCHAR];
} control_struct;
