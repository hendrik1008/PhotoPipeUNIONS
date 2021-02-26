
typedef struct {
    double	crval1, crval2, cdelt1, cdelt2, crpix1, crpix2;
    double	pc11, pc12, pc21, pc22;
    int		xwid, ywid, chan_nr;
} field_params;

extern field_params **load_field_description(tabstruct *, int *,
           char *, char *, char *, char *, char *, char *, char *, char *);

void free_field_desc(field_params **p, int n);
