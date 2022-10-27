extern char	*strip_space(char *),
		*strip_name(int),
		*leading_zeros(int, int),
		**uniq_strings(char *, int, int *),
		*decode_path(char *, int *),
		*to_lower(char *),
		*to_upper(char *);


extern int	match(char *, char *),
		*uniq_int(int *, int *),
		**uniq_2_int(int **, int *),
                string_in_list(char *s, char **t, int n),
		strcmpcu(char *, char *); /* Compare uppercase */

