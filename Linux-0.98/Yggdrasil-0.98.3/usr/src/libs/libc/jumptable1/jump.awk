BEGIN   {
		print "\t.text"
		print "\t.long VERSION\n"
	}

	{
		printf "\t.globl\t_%s__jump\n", $1
		printf "\t.align\t2,0x90\n"
		printf "%s__jump:\n", $1
		printf "\tjmp\t_%s\n\n", $1
	}

END	{
		printf "\t.align\t14,0x90\n"
	}
