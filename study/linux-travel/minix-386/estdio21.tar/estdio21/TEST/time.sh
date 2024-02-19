#/bin/sh

echo -n "ten	"
echo `/bin/time ./ten 256 2>&1 >/dev/null`
echo -n "putc	"
echo `/bin/time ./putc 256 2>&1 >/dev/null`
echo -n "fputs	"
echo `/bin/time ./fputs 256 2>&1 >/dev/null`
echo -n "nfputs	"
echo `/bin/time ./nfputs 256 2>&1 >/dev/null`
echo -n "fwrite	"
echo `/bin/time ./fwrite 256 2>&1 >/dev/null`
echo -n "sprintf	"
echo `/bin/time ./sprintf 256 2>&1 >/dev/null`
echo -n "printf	"
echo `/bin/time ./printf 256 2>&1 >/dev/null`
echo -n "nprintf	"
echo `/bin/time ./nprintf 256 2>&1 >/dev/null`
echo -n "printfs	"
echo `/bin/time ./printfs 256 2>&1 >/dev/null`
echo -n "putchar	"
echo `/bin/time ./putchar 256 2>&1 >/dev/null`
