#ifndef VDL_UTILS_H
#define VDL_UTILS_H

#include <sys/types.h>
#include <stdarg.h>
#include "vdl.h"

void vdl_utils_linkmap_print (void);


// allocate/free memory
void *vdl_utils_malloc (size_t size);
void vdl_utils_free (void *buffer, size_t size);
#define vdl_utils_new(type) \
  (type *) vdl_utils_malloc (sizeof (type))
#define vdl_utils_delete(v) \
  vdl_utils_free (v, sizeof(*v))

// string manipulation functions
int vdl_utils_strisequal (const char *a, const char *b);
int vdl_utils_strlen (const char *str);
void vdl_utils_strfree (char *str);
char *vdl_utils_strdup (const char *str);
char *vdl_utils_strfind (char *str, const char *substr);
void vdl_utils_memcpy (void *dst, const void *src, size_t len);
void vdl_utils_memset(void *s, int c, size_t n);
char *vdl_utils_strconcat (const char *str, ...);
const char *vdl_utils_getenv (const char **envp, const char *value);

// convenience function
int vdl_utils_exists (const char *filename);

// manipulate string lists.
struct VdlStringList *vdl_utils_strsplit (const char *value, char separator);
void vdl_utils_str_list_free (struct VdlStringList *list);
struct VdlStringList *vdl_utils_str_list_reverse (struct VdlStringList *list);
struct VdlStringList * vdl_utils_str_list_append (struct VdlStringList *start, 
						  struct VdlStringList *end);
struct VdlStringList * vdl_utils_str_list_prepend (struct VdlStringList *start, 
						   struct VdlStringList *end);
struct VdlStringList * vdl_utils_str_list_split (struct VdlStringList *start, 
						 struct VdlStringList *at);
struct VdlStringList *vdl_utils_splitpath (const char *value);

unsigned long vdl_utils_align_down (unsigned long v, unsigned long align);
unsigned long vdl_utils_align_up (unsigned long v, unsigned long align);

#define vdl_utils_max(a,b)(((a)>(b))?(a):(b))

ElfW(Phdr) *vdl_utils_search_phdr (ElfW(Phdr) *phdr, int phnum, int type);

char *vdl_utils_vprintf (const char *str, va_list args);

#endif /* VDL_UTILS_H */
