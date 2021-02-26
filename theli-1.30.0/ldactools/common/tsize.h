/*
   tsize.h
   (created 05.05.03; Thomas Erben)
*/

/* this file contains the size in bytes of data
   types defined in the enum type t_type. The
   sizes are in an extra file to avoid compiler
   warnings due to the definition of the
   int array t_size.
*/

#ifndef __TSIZE_H__
#define __TSIZE_H__

#ifdef __cplusplus
extern "C"
{
#endif

static int	t_size[] = {1, 2, 4, 4, 8, 1};	/* size in bytes per t_type */

#ifdef __cplusplus
}
#endif

#endif

