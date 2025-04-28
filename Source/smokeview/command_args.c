/// @file command_args.

// This file explicitly does not include "smokeviewcars.h" to avoid separate
// this parsing from global variables.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"
#include "dmalloc.h"
#include "command_args.h"

/* ------------------ CLE_Message ------------------------ */

const char *CLE_Message(enum CommandLineError cle, char *message) {
  char message_local[80];
  switch (cle) {
  case CLE_OK:
    return NULL;
    break;
  case CLE_MULTIPLE_LANG:
    return "***error: -lang defined multiple times";
    break;
  case CLE_NO_LANG:
    return "***error: -lang must be given an argument";
    break;
  case CLE_MULTIPLE_INPUTS:
    return "***error: multiple input files found";
  case CLE_UNKNOWN_ARGUMENT:
    strcpy(message_local, "***error: ");
    strcat(message_local, message);
    strcat(message_local, " is an unknown parameter");
    strcpy(message, message_local);
    return message;
    break;
  default:
    return "***error: a commandline parsing error occurred";
    break;
  }
}

/* ------------------ ParseCommandlineNew ------------------------ */

CommandlineArgs ParseCommandlineNew(int argc, char **argv, char *message,
                                    enum CommandLineError *error) {
  CommandlineArgs args = {0};
  *error = CLE_OK;
  if (argc >= 1) {
    NewMemory((void **)&args.prog, strlen(argv[0]) + 1);
    strcpy(args.prog, argv[0]);
  }
  int i;
#ifdef pp_OSX_HIGHRES
  args.x1 = true;
  args.x2 = false;
#endif
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-ini") == 0) {
      args.ini = true;
    } else if (strcmp(argv[i], "-ng_ini") == 0) {
      args.ng_ini = true;
    } else if (strcmp(argv[i], "-volrender") == 0) {
      args.volrender = true;
    }
    else if (strcmp(argv[i], "-version") == 0 || strcmp(argv[i], "-v") == 0) {
      args.print_version = true;
    }
#ifdef pp_OSX_HIGHRES
    else if (strcmp(argv[i], "-1x") == 0) {
      args.x1 = true;
      args.x2 = false;
    } else if (strcmp(argv[i], "-2x") == 0) {
      args.x1 = false;
      args.x2 = true;
    }
#endif
    else if (strcmp(argv[i], "-update_bounds") == 0) {
      args.update_bounds = true;
    } else if (strcmp(argv[i], "-no_graphics") == 0) {
      args.no_graphics = true;
    } else if (strcmp(argv[i], "-update_slice") == 0) {
      args.update_slice = true;
    } else if (strcmp(argv[i], "-update") == 0) {
      args.update = true;
    } else if (strcmp(argv[i], "-nogpu") == 0) {
      args.nogpu = true;
    } else if(strcmp(argv[i], "-check_colorbar") == 0) {
      args.check_colorbar = true;
    } else if (strcmp(argv[i], "-demo") == 0) {
      args.demo = true;
    } else if (strcmp(argv[i], "-info") == 0) {
      args.info = true;
    } else if (strcmp(argv[i], "-sizes") == 0) {
      args.sizes = true;
    } else if (strcmp(argv[i], "-stereo") == 0) {
      args.stereo = true;
    } else if (strcmp(argv[i], "-big") == 0) {
      args.big = true;
    } else if(strcmp(argv[i], "-trirates") == 0) {
      args.trirates = true;
    } else if (strcmp(argv[i], "-timings") == 0) {
      args.timings = true;
    } else if (strcmp(argv[i], "-lang") == 0) {
      // The next argument is a language name
      ++i;
      if (i < argc) {
        if (args.lang != NULL) {
          *error = CLE_MULTIPLE_LANG;
          return args;
        }
        // Why the additional 48 bytes?
        NewMemory((void **)&args.lang, strlen(argv[i]) + 48 + 1);
        strcpy(args.lang, argv[i]);
      } else {
        *error = CLE_NO_LANG;
        return args;
      }
    } else if (strcmp(argv[i], "-convert_ini") == 0) {
      args.convert_ini = true;
      // The next two arguments are the from and to paths for ini conversion.
      char *local_ini_from = NULL, *local_ini_to = NULL;

      if (++i < argc) {
        local_ini_from = argv[i];
      } else {
        *error = CLE_NO_INI_ARGS;
        return args;
      }
      if (++i < argc) {
        local_ini_to = argv[i];
      } else {
        *error = CLE_NO_INI_ARGS;
        return args;
      }
      if (local_ini_from != NULL && local_ini_to != NULL) {
        NewMemory((void **)&args.ini_from, strlen(local_ini_from) + 1);
        strcpy(args.ini_from, local_ini_from);

        NewMemory((void **)&args.ini_to, strlen(local_ini_to) + 1);
        strcpy(args.ini_to, local_ini_to);
      }
    } else if (strcmp(argv[i], "-convert_ssf") == 0) {
      args.convert_ssf = true;
      // The next two arguments are the from and to paths for ssf conversion.
      char *local_ssf_from = NULL, *local_ssf_to = NULL;

      if (++i < argc) {
        local_ssf_from = argv[i];
      } else {
        *error = CLE_NO_SSF_ARGS;
        return args;
      }
      if (++i < argc) {
        local_ssf_to = argv[i];
      } else {
        *error = CLE_NO_SSF_ARGS;
        return args;
      };
      if (local_ssf_from != NULL && local_ssf_to != NULL) {
        NewMemory((void **)&args.ssf_from, strlen(local_ssf_from) + 1);
        strcpy(args.ssf_from, local_ssf_from);

        NewMemory((void **)&args.ssf_to, strlen(local_ssf_to) + 1);
        strcpy(args.ssf_to, local_ssf_to);
      }
    } else if (strcmp(argv[i], "-update_ssf") == 0) {
      args.update_ssf = true;
    } else if (strcmp(argv[i], "-update_ini") == 0) {
      // As per convert_ini, but in-place
      args.update_ini = true;
      char *local_ini_from = NULL, *local_ini_to = NULL;

      if (++i < argc) {
        local_ini_from = argv[i];
      } else {
        *error = CLE_NO_INI_ARGS;
        return args;
      }
      local_ini_to = local_ini_from;
      if (local_ini_from != NULL) {
        NewMemory((void **)&args.ini_from, strlen(local_ini_from) + 1);
        strcpy(args.ini_from, local_ini_from);

        NewMemory((void **)&args.ini_to, strlen(local_ini_to) + 1);
        strcpy(args.ini_to, local_ini_to);
      }
    } else if (strcmp(argv[i], "-isotest") == 0) {
      args.isotest = true;
    } else if (strcmp(argv[i], "-smoke3d") == 0) {
      args.smoke3d = true;
    } else if (strcmp(argv[i], "-no_slcf") == 0) {
      args.no_slcf = true;
    } else if (strcmp(argv[i], "-h") == 0 &&
               strcmp(argv[i], "-help_all") != 0 &&
               strcmp(argv[i], "-html") != 0) {
      args.show_help_summary = true;
    } else if (strcmp(argv[i], "-help_all") == 0) {
      args.show_help_all = true;
    } else if (strcmp(argv[i], "-noblank") == 0) {
      args.noblank = true;
    }
    else if(strcmp(argv[i], "-nobounds") == 0) {
      args.nobounds = true;
    }
    else if (strcmp(argv[i], "-verbose") == 0) {
      args.verbose = true;
    } else if (strcmp(argv[i], "-outline") == 0) {
      args.outline = true;
    } else if (strcmp(argv[i], "-make_movie") == 0) {
      args.make_movie = true;
    } else if (strcmp(argv[i], "-csv") == 0) {
      args.csv = true;
    } else if(strcmp(argv[i], "-max_mem") == 0) {
      args.max_mem = true;
      i++;
      sscanf(argv[i],"%f",&args.max_mem_GB);
      if(args.max_mem_GB<0.0)args.max_mem_GB = 0.0;
    }
    else if(strcmp(argv[i], "-x0") == 0) {
      args.have_x0 = true;
      i++;
      sscanf(argv[i],"%i",&args.x0);
    }
    else if(strcmp(argv[i], "-y0") == 0) {
      args.have_y0 = true;
      i++;
      sscanf(argv[i],"%i",&args.y0);
    }
    else if(strcmp(argv[i], "-X0") == 0) {
      args.have_X0 = true;
      i++;
      sscanf(argv[i], "%i", &args.X0);
    }
    else if(strcmp(argv[i], "-Y0") == 0) {
      args.have_Y0 = true;
      i++;
      sscanf(argv[i], "%i", &args.Y0);
    }
    else if (strcmp(argv[i], "-geominfo") == 0) {
      args.geominfo = true;
    } else if (strcmp(argv[i], "-fast") == 0) {
      args.fast = true;
    }
    else if(strcmp(argv[i], "-load_co2") == 0) {
      args.load_co2 = true;
    }
    else if(strcmp(argv[i], "-load_hrrpuv") == 0) {
      args.load_hrrpuv = true;
    }
    else if(strcmp(argv[i], "-load_soot") == 0) {
      args.load_soot = true;
    }
    else if(strcmp(argv[i], "-load_temp") == 0) {
      args.load_temp = true;
    }
    else if(strcmp(argv[i], "-full") == 0) {
      args.full = true;
    } else if (strcmp(argv[i], "-blank") == 0) {
      args.blank = true;
    } else if (strcmp(argv[i], "-gversion") == 0) {
      args.gversion = true;
    } else if (strcmp(argv[i], "-redirect") == 0) {
      args.redirect = true;
    } else if (strcmp(argv[i], "-runscript") == 0) {
      args.runscript = true;
    } else if (strcmp(argv[i], "-runhtmlscript") == 0) {
      args.runhtmlscript = true;
    }
    else if (strcmp(argv[i], "-scriptrenderdir") == 0) {
      i++;
      if (i < argc) {
        NewMemory((void **)&args.scriptrenderdir, strlen(argv[i]) + 1);
        strcpy(args.scriptrenderdir, argv[i]);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else if (strcmp(argv[i], "-skipframe") == 0) {
      args.skipframe_defined = true;
      ++i;
      if (i < argc) {
        sscanf(argv[i], "%i", &args.skipframe);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else if (strcmp(argv[i], "-startframe") == 0) {
      args.startframe_defined = true;
      ++i;
      if (i < argc) {
        sscanf(argv[i], "%i", &args.startframe);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else if (strcmp(argv[i], "-volrender") == 0) {
      args.volrender = true;
    } else if (strcmp(argv[i], "-script") == 0) {
      ++i;
      if (i < argc) {
        NewMemory((void **)&args.script, strlen(argv[i]) + 1);
        strcpy(args.script, argv[i]);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else if (strcmp(argv[i], "-checkscript") == 0) {
      ++i;
      if (i < argc) {
        NewMemory((void **)&args.script, strlen(argv[i]) + 1);
        strcpy(args.script, argv[i]);
        args.checkscript = true;
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        void SMV_EXIT(int error);
        SMV_EXIT(1);
        return args;
      }
    }
    else if (strcmp(argv[i], "-htmlscript") == 0) {
      ++i;
      if (i < argc) {
        NewMemory((void **)&args.htmlscript, strlen(argv[i]) + 1);
        strcpy(args.htmlscript, argv[i]);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else if (strcmp(argv[i], "-noexit") == 0) {
      args.noexit = true;
    } else if (strcmp(argv[i], "-setup") == 0) {
      args.setup = true;
    } else if(strcmp(argv[i], "-large") == 0) {
      args.large_case = true;
    } else if (strcmp(argv[i], "-bindir") == 0) {
      ++i;
      if (i < argc) {
        NewMemory((void **)&args.bindir, strlen(argv[i]) + 1);
        strcpy(args.bindir, argv[i]);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else if (strcmp(argv[i], "-casedir") == 0) {
      ++i;
      if (i < argc) {
        NewMemory((void **)&args.casedir, strlen(argv[i]) + 1);
        strcpy(args.casedir, argv[i]);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else if (strcmp(argv[i], "-threads") == 0) {
      args.threads_defined = true;
      ++i;
      if (i < argc) {
        sscanf(argv[i], "%i", &args.threads);
      } else {
        *error = CLE_ARGUMENT_EXPECTED;
        return args;
      }
    } else {
      if(argv[i][0] == '-') {
        *error = CLE_UNKNOWN_ARGUMENT;
        strcpy(message, argv[i]);
        return args;
      }
      if (args.input_file == NULL) {
        NewMemory((void **)&args.input_file, strlen(argv[i]) + 1);
        strcpy(args.input_file, argv[i]);
      } else {
        *error = CLE_MULTIPLE_INPUTS;
        return args;
      }
    }
  }
  return args;
}

/// @brief Free any allocated buffers in @ref CommandlineArgs. Performs
/// null-checks on all buffers.
/// @param args The previously parse command line arguments.

/* ------------------ FreeCommandlineArgs ------------------------ */

void FreeCommandlineArgs(CommandlineArgs *args) {
  FREEMEMORY(args->prog);
  FREEMEMORY(args->input_file);
  FREEMEMORY(args->bindir);
  FREEMEMORY(args->casedir);
  FREEMEMORY(args->lang);
  FREEMEMORY(args->ini_from);
  FREEMEMORY(args->ini_to);
  FREEMEMORY(args->ssf_from);
  FREEMEMORY(args->ssf_to);
}
