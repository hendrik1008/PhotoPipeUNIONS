/*
 *
 * 15.06.2001: removed the #define constructs around VPRINTF
 *             keeping only the first definition works on all
 *             platforms so far.
 *
 * 22.07.01: surrounded the whole include file by an #ifdef
 *           construct to prevent multiple inclusions
 */

/****** options ***********************************************************
 *
 *  INTRODUCTION
 *      The options package is used to allow run-time configuration of
 *      programs. Through a configuration file description many paramaters
 *      can be set. These parameters are defined through an option
 *      structure by the calling program. The layout of the option
 *      structure is given below.
 *
 *      The configuration file syntax is very simple. It is described
 *      below.
 *
 *  USAGE
 *      To allow usage of this package one needs to include the following
 *      file in the source code:
 *         #include "options_globals.h"
 *
 *      There are two main and one auxiliary routine in the package. The
 *      first routine is the function that does the actual interpretation
 *      of a configuration file on the basis of user provided options
 *      (parse_options). The other routine makes  use of the verbosity level
 *      as it is set by the user in the configuration file. Therefor the
 *      VPRINTF function can only be used after the parse_options call.
 *      The auxiliary routine verbosity allows the caller to see if the
 *      current verbosity level exceeds the arguments verbosity. This
 *      allows routine segments to be run on the basis of the verbosity
 *      level.
 *
 *  CONFIGURATION FILE
 *      The configuration file syntax is very simple and has UNIX like
 *      properties. The parsing of the file is done on a line by line
 *      basis, so all information pertaining to a particular option need
 *      to be on that line. The option name and ist values are separated by
 *      and equals (=) sign, optionally encompassed in spaces or TAB's.
 *      A line that starts with a hash (#) sign is considered a comment
 *      line. In line comments (comments on the same line as option
 *      information) is not allowed.
 *
 *      Thus a few lines from a configuration file could look like:
 *      # This is a comment
 *      OPTION1 = VALUE1
 *      OPTION2=	VALUE2
 *
 *  OPTIONS STRUCTURE
 *      The options structure definition contains several different types
 *      of data. The currently available types are:
 *        BOOLEAN, FLOAT, INT, STRING, IARRAY, FARRAY
 *      of which the latter two are integer and float arrays, respectively.
 *      For a BOOLEAN the allowed values are YES, and NO. The option
 *      structure looks like:
 *      typedef struct {
 *              char   name[OPTLEN];          Name of the option
 *              mem_type type;                Type of the option
 *              void   *ptr;                  Pointer to the option data
 *              int    imin, imax;            Minimun and maximum int values
 *              double dmin, dmax;            Minimun and maximum float values
 *              char   list[MAXOPTS][OPTLEN]; Array of allowed strings
 *              int    ival;                  Default int value
 *              double dval;                  Default float value
 *              bool   bval;                  Default boolean value
 *              char   cval[OPTLEN];          Default string value
 *      }
 *
 *      It is general practice to create a control structure in the caller
 *      program that describes the user defined option data containers. In
 *      such a case a user provided option list could look like:
 *
 *      typedef struct {
 *          char   option1[OPTLEN];
 *          bool   option2;
 *          double options3;
 *      } control_struct;
 *      option my_opts[NOPTIONS] = {
 *          {"OPTION1", STRING, control.option1, 0, 0, 0.0, 0.0,
 *            0, 0.0, NO, "SMART",
 *           "DUM", "SMART", "IGNORANT", "SCHIZO", ""},
 *          {"OPTION2", BOOLEAN, control.option2, 0, 0, 0.0, 0.0,
 *            0, 0.0, YES, "",
 *           ""},
 *          {"OPTION3", FLOAT, control.option3, 0, 0, 0.0, 100.0,
 *            0, 10.3, NO, "",
 *           ""}
 *          };
 *
 *      Above definition creates three allowed options, the first of type
 *      string with allowed values "DUM", "SMART", "IGNORANT", and "SCHIZO"
 *      and default value "SMART". Note that all strings are in upper case.
 *      The parse_options routine is case insensitive. The second option
 *      is of type boolean, with default value YES. And the last option
 *      is a floating type with allowed range of values between 0 and 100,
 *      and with default value 10.3.
 *
 *      The user should not provide a VERBOSE option as this will always be
 *      added by the parse_options routine. In fact if the use does provide
 *      such an option, that will hide the system VERBOSE option and thus
 *      cause the VPRINTF and verbosity routines to malfunction.
 *
 *  AUTHOR
 *      E.R Deul  - dd 25-09-1994 (Original version)
 *                  dd 21-02-1995 (Loosely decoupled version)
 *                  dd 16-02-1996 (Completely decoupled version)
 *
 ******************************************************************************/

#ifndef __OPTIONS_GLOBALS__
#define __OPTIONS_GLOBALS__

#include <stdio.h>
#include <stdarg.h>
#include "fitscat.h"
#include "options_defs.h"
#include "options_types.h"

extern void	parse_options(char *),
		altverbose(char *),
                init_options(int, option *),
                print_options(char *),
                finish_options(),
		altverbosity(char *),
                set_command_option(char *, char *),
                VPRINTF(int, const char *, ...);

extern int	verbosity(int);

extern int	save_options(tabstruct *, option *, int);

extern char	**copy_textvec(char *, int *);

#endif
