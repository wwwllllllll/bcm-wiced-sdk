#include <stdio.h>

int main(int argc, char* argv[])
{
	int i = 0;
	int total_bytes = 0;
	while ( (i < argc) && ((total_bytes +  strlen(argv[i]) + 1) < 8190 ) )
	{
		printf("%s ", argv[i]);
		total_bytes+= strlen(argv[i]) + 1;
		i++;
	}
	return 0;
}