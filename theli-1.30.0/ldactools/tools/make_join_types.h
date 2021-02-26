typedef struct {
    char	conffile[MAXCHAR];
    char	listfile[MAXCHAR];
    char	outfile[MAXCHAR];
    char	table_name_ref[MAXCHAR], table_name_main[MAXCHAR];
    char	name[TEXTVECLEN];
    char	input[TEXTVECLEN];
    char	col_ref[MAXCHAR];
} control_struct;
