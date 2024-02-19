/*                                 e x i t                                 */

#include "stdiolib.h"

/*LINTLIBRARY*/

void exit F1(int, status)

{
  if (__Zatexit != (void (*) P((void))) NULL)
    (*__Zatexit)();
  if (__Zwrapup != (void (*) P((void))) NULL)
    (*__Zwrapup)();
  _exit(status);
}
