#include <stdio.h>
#include <string.h>

int main()
{
	int cases, sum[26], i, max;
	char str[1001];

	scanf("%d", &cases);
	while (cases > 0) {
		scanf("%s", str);
		for (i = 0; i < 26; i++)
			sum[i] = 0;
		for (i = 0; i < strlen(str); i++)
			sum[str[i] - 'a']++;
		max = 0;
		for (i = 1; i < 26; i++)
		if (sum[i] > sum[max]) max = i;
		printf("%c %d\n", max + 'a', sum[max]);
		cases--;
	}
	return 0;
}