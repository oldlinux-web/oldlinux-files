/* mknod - build a special file		Author: Andy Tanenbaum */

main(argc, argv)
int argc;
char *argv[];
{
/* mknod name b/c major minor [size] makes a node. */

  int mode, major, minor, dev;
  unsigned int size;

  if (argc < 5) badcomm();
  if (*argv[2] != 'b' && *argv[2] != 'c') badcomm();
  if (*argv[2] == 'c' && argc != 5) badchar();
  if (*argv[2] == 'b' && argc != 6) badblock();
  mode = (*argv[2] == 'b' ? 060666 : 020666);
  major = atoi(argv[3]);
  minor = atoi(argv[4]);
  size = (*argv[2] == 'b' ? atoi(argv[5]) : 0);
  if (major < 0 || minor < 0) badcomm();
  dev =  (major << 8) | minor;
  if (mknod(argv[1], mode, dev, size) < 0) perror("mknod");
  exit(0);
}

badcomm()
{
  std_err("Usage: mknod name b/c major minor [size_in_blocks]\n");
  exit(1);
}

badchar()
{
  std_err("Usage: mknod name c major minor\n");
  exit(1);
}

badblock()
{
  std_err("Usage: mknod name b major minor size_in_blocks\n");
  exit(1);
}

