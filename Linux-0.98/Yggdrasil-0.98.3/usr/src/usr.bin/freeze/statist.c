#include "freeze.h"
#include "lz.h"

/* This program calculates the distribution of the matched strings'
positions and lengths using nearly the same code as `freeze'.
*/

#define N_POS 62
#define T (N_POS * 2 - 1)
#define R (T - 1)

#define update(c) (freq[c]++)

long in_count, refers = 0;

long indc_count;
short reduceflag = 0, greedy = 0;

int lens[F2+1];

us_t bits[9];

short   prnt[T];
ul_t freq[T];
short used[T];

void freeze(), StartHuff();

SIGTYPE giveres();

int main(argc, argv) char ** argv; {
	argv++;
	while (argc > 1) {
		if (**argv == '-') {
			while (*++(*argv) == 'g')
				greedy++;
			if (**argv)
				goto usage;
			argc--; argv++;
		} else
			break;
	}
	usage:
	if(argc != 1) {
		fprintf(stderr, "Usage: statist [-g...] < sample_file\n");
		fprintf(stderr, "Press INTR to display current values\n");
		exit(0);
	}
	signal(SIGINT, giveres);

#ifdef DOS
	setmode(fileno(stdin), O_BINARY);       /* Oh this MS-DOS ... */
#endif  /* DOS */

	freeze();
	giveres();
	return 0;
}

ul_t isqrt(val)
ul_t val;
{
  ul_t result = 0;
  ul_t side = 0;
  ul_t left = 0;
  int digit = 0;
  int i;
  for (i=0; i<sizeof(ul_t)*4; i++)
  {
    left = (left << 2) + (val >> (sizeof(ul_t) * 8 - 2));
    val <<= 2;
    if (left >= side*2 + 1)
    {
      left -= side*2+1;
      side = (side+1)*2;
      result <<= 1;
      result |= 1;
    }
    else
    {
      side *= 2;
      result <<= 1;
    }
  }
  return result;
}


/* Prints the (current) values of tunable parameters. Uncertainty is
the number of missequencings (algorithm assumes the probabilities
of references decrease uniformly when distance increases). Ideally
it should be 0, but somewhat about 5 or less denotes the given 8 values
could improve the compression rate when using them.
*/

SIGTYPE giveres() {
	us_t c;
	register int i, j, k, pr, f, average, sum;
	ul_t cumul, sigma2;
	short r, percent;
	signal(SIGINT, giveres);
	newtry:
	StartHuff(N_POS);
	pr = f = 0;
	i = N_POS;
	r = N_POS * 2 - 2;
	while (i <= r) {
		j = findmin(i);
		k = findmin(i);
		freq[i] = freq[j] + freq[k];
		prnt[j] = prnt[k] = i++;
	}

	for (c = 1; c <= 6; c++) bits[c] = 0;

	for(c = 0; c < N_POS; c++) {
		j = 0;
		k = c;
		do j++; while ((k = prnt[k]) != r);
		if (j <= 6)
			bits[j]++;
		if (j < pr)
			f += pr - j;
		else
			pr = j;
	}

	k = bits[1] + bits[2] + bits[3] + bits[4] +
	bits[5] + bits[6];

	k = 62 - k;     /* free variable length codes for 7 & 8 bits */

	j = 128 * bits[1] + 64 * bits[2] + 32 * bits[3] +
	16 * bits[4] + 8 * bits[5] + 4 * bits[6];

	j = 256 - j;    /* free byte images for these codes */

/*      Equation:
	    bits[7] + bits[8] = k
	2 * bits[7] + bits[8] = j
*/
	j -= k;
	if (j < 0 || k < j) {
		printf("Huffman tree has more than 8 levels, reducing...\n");
		for (i = 0; i < N_POS; i++)
			if (!freq[i])
				freq[i] = 1;
			else if (reduceflag)
				freq[i] = (freq[i] + 1) / 2;
		reduceflag = 1;
		goto newtry;
	} else {
		bits[7] = j;
		bits[8] = k - j;
		printf("%d %d %d %d %d %d %d %d (uncertainty = %d)\n",
			bits[1], bits[2], bits[3], bits[4],
			bits[5], bits[6], bits[7], bits[8], f);
	}
	sum = 0; cumul = 0;
	for(i = 3; i <= F2; i++) {
		cumul += (ul_t) i * lens[i];
		sum += lens[i];
	}
	sum || sum++;
	printf("Average match length: %d.%02d\n",
		average = cumul / sum, i = cumul * 100 / sum % 100);
	if (i >= 50) average++;
	j = sum;
	percent = 0;
	for (i = F2; i >= 3; i--) {
		static pcs[] = { 999, 995, 990, 970, 950, 900, 800, 700, 500 };
		j -= lens[i];
		newpcs:
		if (j <= sum * pcs[percent] / 1000) {
			printf("Percentile %d.%d: %d\n",
				pcs[percent] / 10, pcs[percent] % 10, i);
			if (percent == sizeof(pcs)/sizeof(int) - 1)
				break;
			else {
				percent++;
				goto newpcs;
			}
		}
	}
	for (sigma2 = 0, i = 3; i <= F2; i++)
		sigma2 += (ul_t)(i - average)*(i - average)*lens[i];
	sigma2 = sigma2 * 100 / sum;
	j = (int)isqrt(sigma2);
	printf("Sigma: %d.%1d\n", j / 10, j % 10);
	printf("References: %ld\n", refers);
	fflush(stdout);
}


void freeze ()
{
	register us_t i, len, r, s;
	register short c;
	StartHuff(0);
	InitTree();
	s = 0;
	r = N2 - F2;
	for (i = s; i < r; i++)
		text_buf[i] = ' ';
	for (len = 0; len < F2 && (c = getchar()) != EOF; len++)
		text_buf[r + len] = c;
	in_count = len;
	for (i = 0; i <= F2; i++)
		InsertNode(r + i - F2);
	while (len != 0) {
		match_length = THRESHOLD;
		Get_Next_Match(r,1);
		if (match_length > len)
			match_length = len;
		if (match_length <= THRESHOLD) {
			match_length = 1;
		} else if (greedy) {
			lens[match_length] ++;
			update((us_t)match_position >> 7);
			refers ++;
		} else {
			register us_t orig_length, orig_position;
			orig_length = match_length;
			orig_position = match_position;
			DeleteNode(s);
			Next_Char(N2, F2);
			Get_Next_Match(r,2);
			if (match_length > len) match_length = len;
			if (orig_length > match_length) {
				lens[orig_length] ++;
				update((us_t)orig_position >> 7);
				match_length = orig_length - 1;
			} else  {
				lens[match_length] ++;
				update(match_position >> 7);
			}
			refers ++;
		}
		for (i = 0; i < match_length &&
				(c = getchar()) != EOF; i++) {
			DeleteNode(s);
			text_buf[s] = c;
			if (s < F2 - 1)
				text_buf[s + N2] = c;
			s = (s + 1) & (N2 - 1);
			r = (r + 1) & (N2 - 1);
			InsertNode(r);
		}
		in_count += i;
		if ((in_count > indc_count)) {
			fprintf(stderr, "%5dK\b\b\b\b\b\b", in_count / 1024);
			fflush (stderr);
			indc_count += 4096;
		}
		while (i++ < match_length) {
			DeleteNode(s);
			s = (s + 1) & (N2 - 1);
			r = (r + 1) & (N2 - 1);
			if (--len) InsertNode(r);
		}
	}
}

void StartHuff(beg) {
	int i;
	for (i = beg; i < N_POS * 2 - 1; i++)
		freq[i] = 0;
	for (i = 0; i < N_POS * 2 - 1; i++)
		used[i] = prnt[i] = 0;
}

int findmin(range) {
	long min = (1 << 30) - 1, argmin = -1, i;
	for (i = 0; i < range; i++) {
		if(!used[i] && freq[i] < min)
			min = freq[argmin = i];
	}
	used[argmin] = 1;
	return argmin;
}
