//===================================================================================================
//author:L
//date:6,24,2019,Monday,Rain
//=======       =======       =======       =======
//       =     =       =     =       =     =
//        =   =         =   =         =   =
//          =             =             =
//        =   =         =   =         =   =
//       =     =       =     =       =     =
//=======       =======       =======       =======
//===================================================================================================

#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool int
#define true 1
#define false 0

int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;

//===================================================================================================
//读取参数，初始化cache
//===================================================================================================
struct myCachePara{
	int S;
	int E;
	int B;
	int s;
	int b;
	bool verbose;
	char *filename;
};

void usage(char *func){
	printf("Usage: %s [-hv] -s <s> -e <e> -b <b> -t <tracefile>\n", func);
	exit(0);
}

int checkPara(char arg[], char *func){
	//检查是否能读取相应参数
	int len = strlen(arg);
	int temp = 0;
	for(int i=0; i<len; i++){
		if(arg[i] < '0' || arg[i] > '9'){
			usage(func);
		}
		//  数据量太小，不需要考虑溢出
		temp = temp*10 + (arg[i] - '0');
	}
	return temp;
}

struct myCachePara parse(int argc, char* argv[]){
	//解析输入参数，初始化缓存
	if(argc < 2){
		usage(argv[0]);
	}
	struct myCachePara para = {0, 0, 0, 0, 0, false};
	bool s=false, e=false, b=false, t=false;
	for(int i=1; i<argc; ++i){
		if(strlen(argv[i]) != 2 || argv[i][0] != '-'){
			usage(argv[0]);
		}
		char c = argv[i][1];
		switch (c){
			case 'h':
				usage(argv[0]);
			case 'v':
				para.verbose = true;
				continue;
		}
		//segt后面应该接一个参数
		if(++i == argc){
			usage(argv[0]);
		}

		switch(c){
			case 's':
			case 'S':
				s = true;
				para.s = checkPara(argv[i], argv[0]);
				para.S = 1<<para.s;
				break;
			case 'e':
			case 'E':
				e = true;
				para.E = checkPara(argv[i], argv[0]);
				break;
			case 'b':
			case 'B':
				b = true;
				para.b = checkPara(argv[i], argv[0]);
				para.B = 1<<para.b;
				break;
			case 't':
			case 'T':
				t = true;
				para.filename = argv[i];
				break;
			default:
				usage(argv[0]);
		}	
	}
	if((s & e & b & t) == 0){
		usage(argv[0]);
	}
	return para;
}

unsigned int toTen(char *addres){
	//将字符串形式的6进制地址转换为10进制数字
	unsigned int res = 0;
	char c;
	for(char *p = addres; *p != '\0'; p++){
		c = *p;
		if(c >= '0' && c <= '9'){
			res = res * 16 + (c - '0');
		} else if(c >= 'a' && c <= 'z'){
			res = res * 16 + (c - 'a') + 10;
		} else if(c >= 'A' && c <= 'Z'){
			res = res * 16 + (c - 'A') + 10;
		}else{
			printf("Something wrong in decode addres\n");
			printf("Wrong address:%c\n", c);
			exit(3);
		}
	}
	return res;
}
//===================================================================================================
//判断是否命中
//===================================================================================================
void exectOp(int targets[], int used[], int E, int target, char *str){
	int maxUsed = 0;
	int longTimeNoUsed = -1; //最久没用过的项
	int available = -1; //空闲项
	bool hit = false;
	for(int i=0; i<E; i++){
		if(used[i] == -1){
			available = i;
		}
		else if(target == targets[i]){
			used[i] = 0;
			hit = true;
			//命中之后还要继续循环
			//修改后面的时间戳
		}
		else if(++used[i] > maxUsed){
			longTimeNoUsed = i;
			maxUsed = used[i];
		}
	}

	if(hit){
		//hit
		sprintf(str, "%s hit{target=%d}", str, target);
		hit_count++;
	}else{
		sprintf(str, "%s miss{target=%d}", str, target);
		miss_count++;
		if(available == -1){
			//没有空闲项
			sprintf(str, "%s eviction{old target=%d}", str, targets[longTimeNoUsed]);
			eviction_count++;
			used[longTimeNoUsed] = 0;
			targets[longTimeNoUsed] = target;
		}else{
			used[available] = 0;
			targets[available] = target;
		}
	}
}

int main(int argc, char* argv[])
{
	// for(int i=0; i<argc; i++){
	// 	printf("Argument %d is %s\n", i, argv[i]);
	// }

	struct myCachePara para = parse(argc, argv);
	printf("Begin Cache! S=%d  E=%d  B=%d testfile=%s\n", para.S, para.E, para.B, para.filename);

	int targets[para.S][para.E];
	int used[para.S][para.E];//使用时间戳实现LRU
	for(int i=0; i<para.S; ++i){
		for(int j=0; j<para.E; ++j){
			targets[i][j] = -1;
			used[i][j] = -1; //-1表示这项无效
		}
	}

	FILE *fp = fopen(para.filename, "r");
	if(fp == NULL){
		printf("Open file:%s failed!\n", para.filename);
		exit(2);
	}
	char buf[255];
	char c;
	while(fgets(buf, 255, fp) != NULL){
		c = buf[0];
		if(c != 'I'){
			//空格，代表接下来是M or L or S
			//忽略I 
			int sindex, target;
			unsigned int addr;
			char addres[100], str[100], op, size[100];
			// int i=3;
			// for(; i<255; i++){
			// 	if(buf[i] != ','){
			// 		addres[i-3] = buf[i];
			// 	}else{
			// 		addres[i-3] = '\0';
			// 		i++;
			// 		break;
			// 	}
			// }
			// int j = i;
			// for(; i<255; i++){
			// 	if(buf[i] != '\n'){
			// 		size[i-j] = buf[i];
			// 	}else{
			// 		size[i-j] = '\0';
			// 		break;
			// 	}
			// }
			sscanf(buf, " %c %[0-9a-zA-Z],%[0-9]", &op, addres, size);
			addr = toTen(addres);
			sindex = addr>>para.b;
			sindex = sindex & ((1<<para.s) - 1);
			target = addr>>(para.b+para.s);
			sprintf(str, " %c %s,%s", op, addres, size);
			exectOp(targets[sindex], used[sindex], para.E, target, str);
			switch (op){
			case 'L':
			case 'S':			
				break;
			case 'M':
				exectOp(targets[sindex], used[sindex], para.E, target, str);
				break;
			default:
				printf("Unknown:%c   Something wrong in test file!\n", op);
				exit(1);
			}
			if(para.verbose){
				printf("%s s=%d\n", str, sindex);
			}
		}
	}

	fclose(fp);
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
