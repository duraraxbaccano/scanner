//This is a sample program file for a C-like simple scanner 
main()
{	int i, j;
	char ch = 'K';
	i = j = 0;
	j = ch - 'A' + 'a';
	double k = j * 3.5e+25;
	j = 32768;
	while (j) {
		out(j);
		switch (i) {
			case 0: 
				out(i);
			case 1:
				out(j);
			case 2:
				out(k);
		}
		out(i);
		for(;;);
	}
	if (i) 
		out(i);
	else
		out(j);
	out(k);
}
    
