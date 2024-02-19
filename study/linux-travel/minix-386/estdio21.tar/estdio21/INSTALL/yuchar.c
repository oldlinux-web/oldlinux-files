int main()

{
  int x = ~0;
  void exit();

  exit(((int) ((unsigned char) x)) == x);
  return 1;
}
