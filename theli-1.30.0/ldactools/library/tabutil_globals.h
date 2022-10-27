/*
 * 15.07.01:
 * included ifdefs to avoid possible multiple includings
 * and to make it work under C++ (extern "C")
 *
 * 24.01.05:
 * I changed char * arguments in 'add_key_to_tab'
 * to const char * to avoid warnings under C++

 */

#include "fitscat.h"

#ifndef __TABUTIL_GLOB_H__
#define __TABUTIL_GLOB_H__

#ifdef __cplusplus
extern "C"
{
#endif


extern void	clear_keys(tabstruct *);

extern keystruct *add_key_to_tab(tabstruct *,
                                 const char *, int, void *,
                                 t_type, int, const char *,
                                 const char *);

extern int	get_channel_number(tabstruct *);

extern char	*get_channel_name(tabstruct *);

#ifdef __cplusplus
}
#endif

#endif



