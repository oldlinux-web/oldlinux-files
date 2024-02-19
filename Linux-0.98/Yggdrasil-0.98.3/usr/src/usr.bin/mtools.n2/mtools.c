#include <stdio.h>
#include <string.h>

#define DISPATCH(f,n) if (!strcmp(name,n)) { extern f(); \
  return f(argc,argv); }

main(argc,argv)
int argc;
char **argv;
{
    char *name;

    load_devices();
    if (name = strrchr(argv[0],'/')) name++;
    else name = argv[0];
    DISPATCH(mattrib,"mattrib")
    DISPATCH(mcd,"mcd")
    DISPATCH(mcopy,"mcopy")
    DISPATCH(mdel,"mdel")
    DISPATCH(mdir,"mdir")
    DISPATCH(mformat,"mformat")
    DISPATCH(mlabel,"mlabel")
    DISPATCH(mmd,"mmd")
    DISPATCH(mrd,"mrd")
    DISPATCH(mread,"mread")
    DISPATCH(mren,"mren")
    DISPATCH(mtype,"mtype")
    DISPATCH(mwrite,"mwrite")
    fprintf(stderr,"Unknown command %s\n",name);
    return 1;
}
