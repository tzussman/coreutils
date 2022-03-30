/* Atomically swap two files. */

#include <config.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>

#include "system.h"
#include "die.h"
#include "error.h"
#include "quote.h"
#include "renameatu.h"

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "swap"

#define AUTHORS proper_name ("Tal Zussman")

static struct option const longopts[] =
{
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};

/* FILE is an operand of this command. Return FILE's inode number. Report
   an error if there is a problem accessing FILE. */
static ino_t
file_operand (char const *file)
{
  struct stat st;
  if (stat (file, &st) != 0)
    die (EXIT_FAILURE, errno, _("failed to access %s"), quoteaf (file));
  return st.st_ino;
}

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    emit_try_help ();
  else
    {
      printf (_("\
Usage: %s TARGET1 TARGET2\n\
  or:  %s OPTION\n\
"),
              program_name, program_name);
      fputs (_("Atomically exchange TARGET1 and TARGET2.\n"), stdout);
      fputs (HELP_OPTION_DESCRIPTION, stdout);
      fputs (VERSION_OPTION_DESCRIPTION, stdout);
      emit_ancillary_info (PROGRAM_NAME);
    }
  exit (status);
}

int
main (int argc, char **argv)
{
  int c;
  int fail;
  int expected_operands;
  int n_files;
  char **file;

  initialize_main (&argc, &argv);
  set_program_name (argv[0]);
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  atexit (close_stdout);

  while ((c = getopt_long (argc, argv, "", longopts, NULL)) != -1)
    {
      switch (c)
        {
        case_GETOPT_HELP_CHAR;

        case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);

        default:
          usage (EXIT_FAILURE);
        }
    }

  expected_operands = 2;
  n_files = argc - optind;
  file = argv + optind;

  if (n_files < expected_operands)
    {
      if (n_files <= 0)
        error (0, 0, _("missing file operand"));
      else
        error (0, 0, _("missing file operand after %s"),
               quote (argv[argc - 1]));
      usage (EXIT_FAILURE);
    }

  if (expected_operands < n_files)
    {
      error (0, 0, _("extra operand %s"), quote (file[expected_operands]));
      usage (EXIT_FAILURE);
    }

  /* The files share an inode - no need to swap them. */
  if (file_operand(file[0]) == file_operand(file[1]))
    return EXIT_SUCCESS;

  fail = renameatu (AT_FDCWD, file[0], AT_FDCWD, file[1], RENAME_EXCHANGE);

  return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}
