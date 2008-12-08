#include "../alloc.h"
#include <list>

int main (int argc, char *argv[])
{
  struct Alloc alloc;
  alloc_initialize (&alloc);
  int sizes[] = {0, 1, 2, 3, 4, 8, 10, 16, 19, 30, 64, 120, 240, 1020, 
		 4098, 10000, 100000, 1000000};
  for (uint32_t i = 0; i < sizeof (sizes)/sizeof (int); i++)
    {
      int size = sizes[i];
      uint8_t *ptr = alloc_malloc (&alloc, size);
      memset (ptr, 0x66, size);
      alloc_free (&alloc, ptr, size);
    }
  std::list<uint8_t *> ptrs;
  for (uint32_t i = 0; i < sizeof (sizes)/sizeof (int); i++)
    {
      int size = sizes[i];
      uint8_t *ptr = alloc_malloc (&alloc, size);
      memset (ptr, 0x66, size);
      ptrs.push_back (ptr);
    }
  for (uint32_t i = 0; i < sizeof (sizes)/sizeof (int); i++)
    {
      alloc_free (&alloc, ptrs.front (), sizes[i]);
      ptrs.pop_front ();
    }
  ptrs.clear ();

  alloc_destroy (&alloc);


  alloc_initialize (&alloc);

  uint8_t *a = alloc_malloc (&alloc, 32000);
  uint8_t *b = alloc_malloc (&alloc, 2000);
  alloc_free (&alloc, a, 32000);
  alloc_free (&alloc, b, 2000);

  alloc_destroy (&alloc);

  return 0;
}
