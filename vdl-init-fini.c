#include "vdl-init-fini.h"
#include "vdl-utils.h"
#include "vdl-log.h"
#include "vdl-sort.h"


typedef void (*fini_function) (void);

static void
call_fini (struct VdlFile *file)
{
  VDL_LOG_FUNCTION ("file=%s", file->name);
  if (!file->init_called || file->fini_called)
    {
      // if we were never initialized properly or if
      // we are finalized already, no need to do any work
      return;
    }
  // mark the file as finalized
  file->fini_called = 1;

  // Gather information from the .dynamic section
  unsigned long dt_fini = vdl_file_get_dynamic_p (file, DT_FINI);
  unsigned long dt_fini_array = vdl_file_get_dynamic_p (file, DT_FINI_ARRAY);
  unsigned long dt_fini_arraysz = vdl_file_get_dynamic_v (file, DT_FINI_ARRAYSZ);

  // First, invoke the newer DT_FINI_ARRAY functions.
  // The address of the functions to call is stored as
  // an array of pointers pointed to by DT_FINI_ARRAY
  if (dt_fini_array != 0)
    {
      fini_function *fini = (fini_function *) dt_fini_array;
      int i;
      for (i = 0; i < dt_fini_arraysz / sizeof (fini_function); i++, fini++)
	{
	  (*(fini[i])) ();
	}
    }

  // Then, invoke the old-style DT_FINI function.
  // The address of the function to call is stored in
  // the DT_FINI tag, here: dt_fini.
  if (dt_fini != 0)
    {
      fini_function fini = (fini_function) dt_fini;
      fini ();
    }
}


// the glibc elf loader passes all 3 arguments
// to the initialization functions and the libc initializer
// function makes use of these arguments to initialize
// __libc_argc, __libc_argv, and, __environ so, we do the
// same for compatibility purposes.
typedef void (*init_function) (int, char **, char **);

static void
call_init (struct VdlFile *file)
{
  VDL_LOG_FUNCTION ("file=%s", file->name);

  if (file->init_called)
    {
      // we have already been initialized
      return;
    }
  file->init_called = 1;

  if (file->is_executable)
    {
      // The constructors of the main executable are
      // run by the libc initialization code which has
      // been linked into the binary by the compiler.
      // If we run them here, they will be run twice which
      // is not good. So, we just return.
      return;
    }

  // Gather information from the .dynamic section
  unsigned long dt_init = vdl_file_get_dynamic_p (file, DT_INIT);
  unsigned long dt_init_array = vdl_file_get_dynamic_p (file, DT_INIT_ARRAY);
  unsigned long dt_init_arraysz = vdl_file_get_dynamic_v (file, DT_INIT_ARRAYSZ);
  // First, invoke the old-style DT_INIT function.
  // The address of the function to call is stored in
  // the DT_INIT tag, here: dt_init.
  if (dt_init != 0)
    {
      init_function init = (init_function) dt_init;
      init (file->context->argc, file->context->argv, file->context->envp);
    }

  // Then, invoke the newer DT_INIT_ARRAY functions.
  // The address of the functions to call is stored as
  // an array of pointers pointed to by DT_INIT_ARRAY
  if (dt_init_array != 0)
    {
      init_function *init = (init_function *) dt_init_array;
      int i;
      for (i = 0; i < dt_init_arraysz / sizeof (init_function); i++, init++)
	{
	  (*(init[i])) (file->context->argc, file->context->argv, file->context->envp);
	}
    }
}


void vdl_init_fini_call_init (struct VdlFileList *files)
{
  struct VdlFileList *fini_order = vdl_sort_deps_breadth_first (files);
  struct VdlFileList *init_order = vdl_file_list_reverse (fini_order);

  struct VdlFileList *cur;
  for (cur = init_order; cur != 0; cur = cur->next)
    {
      call_init (cur->item);
    }

  vdl_file_list_free (init_order);
}
void vdl_init_fini_call_fini (struct VdlFileList *files)
{
  struct VdlFileList *fini_order = vdl_sort_deps_breadth_first (files);

  struct VdlFileList *cur;
  for (cur = fini_order; cur != 0; cur = cur->next)
    {
      call_fini (cur->item);
    }

  vdl_file_list_free (fini_order);
}
