/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include <stdlib.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose17(int M, int N, int A[N][M], int B[M][N]);
void transpose32(int M, int N, int A[N][M], int B[M][N]);
void transpose64(int M, int N, int A[N][M], int B[M][N]);
void transpose64_3part(int M, int N, int A[N][M], int B[M][N]);
void trans(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == N){
        if(M == 32){
            transpose32(M, N, A, B);
        }else if(M == 64){
            transpose64_3part(M, N, A, B);
        }
        else{
            trans(M, N, A, B);
        }
    }else if(M == 61 && N == 67){
        transpose17(M, N, A, B);
    }else{
        trans(M, N, A, B);
    }
}

int min(int a, int b){
    return a>b?b:a;
}

void transpose17(int M, int N, int A[N][M], int B[M][N]){
    int i, j, t, b;
    for(i = 0; i < N; i += 16)
        for(j = 0; j < M; j += 16)
            for(t = i; t < min(i+16, N); t++)
                for(b = j; b < min(j+16, M); b++){
                        B[b][t] = A[t][b];
                }
}

void transpose64(int M, int N, int A[N][M], int B[M][N]){
    for(int i = 0; i < N; i += 4)
        for(int j = 0; j < M; j += 4){
            int a1=A[i][j], a2=A[i][j+1], a3=A[i][j+2], a4=A[i][j+3];
            int a5=A[i+1][j], a6=A[i+1][j+1], a7=A[i+1][j+2], a8=A[i+1][j+3];
            int a9=A[i+2][j], a10=A[i+2][j+1], a11=A[i+2][j+2], a12=A[i+2][j+3];
            B[j][i] = a1;
            B[j][i+1] = a5;
            B[j][i+2] = a9;
            B[j+1][i] = a2;
            B[j+1][i+1] = a6;
            B[j+1][i+2] = a10;
            B[j+2][i] = a3;
            B[j+2][i+1] = a7;
            B[j+2][i+2] = a11;
            B[j+3][i] = a4;
            B[j+3][i+1] = a8;
            B[j+3][i+2] = a12;
            a1 = A[i+3][j];
            a2 = A[i+3][j+1];
            a3 = A[i+3][j+2];
            a4 = A[i+3][j+3];
            B[j][i+3] = a1;
            B[j+1][i+3] = a2;
            B[j+2][i+3] = a3;
            B[j+3][i+3] = a4;
        }
}

void transpose32(int M, int N, int A[N][M], int B[M][N]){
    for(int i = 0; i < N; i += 8)
        for(int j = 0; j < M; j += 8)
            for(int b = 0; b < 8; b++){
                int a1=A[i+b][j], a2=A[i+b][j+1], a3=A[i+b][j+2], a4=A[i+b][j+3];
                int a5=A[i+b][j+4], a6=A[i+b][j+5], a7=A[i+b][j+6], a8=A[i+b][j+7];
                B[j][i+b] = a1;
                B[j+1][i+b] = a2;
                B[j+2][i+b] = a3;
                B[j+3][i+b] = a4;
                B[j+4][i+b] = a5;
                B[j+5][i+b] = a6;
                B[j+6][i+b] = a7;
                B[j+7][i+b] = a8;    
            }   
}

char transpose64_3part_desc[] = "Transpose64 diagonal+orther+temp";
void transpose64_3part(int M, int N, int A[N][M], int B[M][N]){
    //左上角四个，对应缓存项0 8 16 24的缓存项作为中间值
    int a1, a2, a3, a4, a5, a6, a7, a8, i, j, t, b;
    for(i = 8; i < N; i += 8)
        for(j = 8; j < N; j += 8)
        if(i != j){
            //处理8×8中的左上4×4小矩阵
            for(t = 0; t < 4; t++)
                for(b = 0; b < 4; b++){
                    B[b+j][t+i] = A[t+i][b+j];
                    //A上面4×8的小矩阵，左边4×4直接转置，右边放到B[0-4][0-4]保存
                    B[t][b] = A[t+i][j+b+4];
                }
            for(t = 0; t < 4; t++)
                for(b = 0; b < 4; b++){
                    //一个缓存 八个数字，所以B[b+j[t+t+4]还在缓存中
                    B[b+j][t+i+4] = A[t+i+4][b+j];
                }
            for(t = 0; t < 4; t++)
                for(b = 0; b < 4; b++){
                    B[b+j+4][t+i] = B[t][b];
                    B[b+j+4][t+i+4] = A[t+i+4][b+j+4];
                }
        }
        else{
            //对角线A和B会冲突
            //A上面两个4×4快
            for(t = 0; t < 4; t++)
                for(b = 0; b < 8; b++)
                    B[t][b] = A[i+t][j+b];

            for(t = 0; t < 4; t++)
                for(b = 0; b < 4; b++)
                    B[j+b][i+t] = B[t][b];

            for(t = 0; t < 4; t++)
                for(b = 4; b < 8; b++)
                    B[j+b][i+t] = B[t][b];

            //A下面两个4×4块
             for(t = 4; t < 8; t++)
                for(b = 0; b < 8; b++)
                    B[t][b] = A[i+t][j+b];

            for(t = 4; t < 8; t++)
                for(b = 0; b < 4; b++)
                    B[j+b][i+t] = B[t][b];

            for(t = 4; t < 8; t++)
                for(b = 4; b < 8; b++)
                    B[j+b][i+t] = B[t][b];

          }
    
    //处理第一列8个8×8块
    //处理第一个对角块
    i=0;
    j=0;
    for(t = 0; t < 4; t++){
        a1=A[i+t][j], a2=A[i+t][j+1], a3=A[i+t][j+2], a4=A[i+t][j+3];
        a5=A[i+t][j+4], a6=A[i+t][j+5], a7=A[i+t][j+6], a8=A[i+t][j+7];
        B[j][i+t] = a1;
        B[j+1][i+t] = a2;
        B[j+2][i+t] = a3;
        B[j+3][i+t] = a4;
        //为防止冲突，剩下四个先放在右边4×4的子矩阵
        B[j][i+t+4] = a5;
        B[j+1][i+t+4] = a6;
        B[j+2][i+t+4] = a7;
        B[j+3][i+t+4] = a8;
    }
    //B左下矩阵，将右边的子矩阵放到下面
    for(t = 0; t < 4; t++){
        a1 = B[j+t][i+4];
        a2 = B[j+t][i+5];
        a3 = B[j+t][i+6];
        a4 = B[j+t][i+7];
        B[j+4+t][i] = a1;
        B[j+4+t][i+1] = a2;
        B[j+4+t][i+2] = a3;
        B[j+4+t][i+3]= a4;
    }
    //处理B右上、右下两个子矩阵，完全冲突，只能4×4的块来搞。
    //也就是处理A左下和右下两个子矩阵。
    for(b = 0; b < 4; b++){
            a1=A[i+4+b][j], a2=A[i+4+b][j+1], a3=A[i+4+b][j+2], a4=A[i+4+b][j+3];
            B[j][i+4+b] = a1;
            B[j+1][i+4+b] = a2;
            B[j+2][i+4+b] = a3;
            B[j+3][i+4+b] = a4;
        }

    for(b = 0; b < 4; b++){
        a1=A[i+4+b][j+4], a2=A[i+4+b][j+5], a3=A[i+4+b][j+6], a4=A[i+4+b][j+7];
        B[j+4][i+4+b] = a1;
        B[j+5][i+4+b] = a2;
        B[j+6][i+4+b] = a3;
        B[j+7][i+4+b] = a4;
    }

    //处理第一列和第一行剩下7×2个8×8块
    j=0;
    for(i = 8; i < M; i += 8){
        for(t = i; t < i + 4; t++)
            for(b = j; b < j + 4; b++){
                B[b][t] = A[t][b];
            }
        for(t = i ; t < i + 4; t++)
            for(b = j + 4; b < j + 8; b++){
                B[b][t] = A[t][b];
            }
        for(t = i + 4; t < i + 8; t++)
            for(b = j + 4; b < j + 8; b++)
                B[b][t] = A[t][b];
        for(t = i + 4; t < i + 8; t++)
            for(b = j; b < j + 4; b++)
                B[b][t] = A[t][b];
    }
    i=0;
    for(j = 8; j < M; j += 8){
        for(t = i; t < i + 4; t++)
            for(b = j; b < j + 4; b++){
                B[b][t] = A[t][b];
            }
        for(t = i ; t < i + 4; t++)
            for(b = j + 4; b < j + 8; b++){
                B[b][t] = A[t][b];
            }
        for(t = i + 4; t < i + 8; t++)
            for(b = j + 4; b < j + 8; b++)
                B[b][t] = A[t][b];
        for(t = i + 4; t < i + 8; t++)
            for(b = j; b < j + 4; b++)
                B[b][t] = A[t][b];
    }
}

char transpose64_diagonal_desc[] = "Transpose64 diagonal+orther";
void transpose64_diagonal(int M, int N, int A[N][M], int B[M][N]){
    for(int i = 0; i < N; i += 8)
        for(int j = 0; j < M; j += 8){
            int a1, a2, a3, a4, a5, a6, a7, a8, t, b;
            if(i == j){   
            //B左上矩阵
            for(t = 0; t < 4; t++){
                a1=A[i+t][j], a2=A[i+t][j+1], a3=A[i+t][j+2], a4=A[i+t][j+3];
                a5=A[i+t][j+4], a6=A[i+t][j+5], a7=A[i+t][j+6], a8=A[i+t][j+7];
                B[j][i+t] = a1;
                B[j+1][i+t] = a2;
                B[j+2][i+t] = a3;
                B[j+3][i+t] = a4;
                //为防止冲突，剩下四个先放在右边4×4的子矩阵
                B[j][i+t+4] = a5;
                B[j+1][i+t+4] = a6;
                B[j+2][i+t+4] = a7;
                B[j+3][i+t+4] = a8;
            }
            //B左下矩阵，将右边的子矩阵放到下面
            for(t = 0; t < 4; t++){
                a1 = B[j+t][i+4];
                a2 = B[j+t][i+5];
                a3 = B[j+t][i+6];
                a4 = B[j+t][i+7];
                B[j+4+t][i] = a1;
                B[j+4+t][i+1] = a2;
                B[j+4+t][i+2] = a3;
                B[j+4+t][i+3]= a4;
            }
            //处理B右上、右下两个子矩阵，完全冲突，只能4×4的块来搞。
            //也就是处理A左下和右下两个子矩阵。
            for(b = 0; b < 4; b++){
                    a1=A[i+4+b][j], a2=A[i+4+b][j+1], a3=A[i+4+b][j+2], a4=A[i+4+b][j+3];
                    B[j][i+4+b] = a1;
                    B[j+1][i+4+b] = a2;
                    B[j+2][i+4+b] = a3;
                    B[j+3][i+4+b] = a4;
                }

             for(b = 0; b < 4; b++){
                    a1=A[i+4+b][j+4], a2=A[i+4+b][j+5], a3=A[i+4+b][j+6], a4=A[i+4+b][j+7];
                    B[j+4][i+4+b] = a1;
                    B[j+5][i+4+b] = a2;
                    B[j+6][i+4+b] = a3;
                    B[j+7][i+4+b] = a4;
                }
            }
            else{
                //  其他块，A和B的缓存项不冲突。
                //但是A和B自身 8×8的块，上下两个4×4的块冲突。
                for(t = i; t < i + 4; t++)
                    for(b = j; b < j + 4; b++){
                        B[b][t] = A[t][b];
                    }
                for(t = i ; t < i + 4; t++)
                    for(b = j + 4; b < j + 8; b++){
                        B[b][t] = A[t][b];
                    }
                for(t = i + 4; t < i + 8; t++)
                    for(b = j + 4; b < j + 8; b++)
                        B[b][t] = A[t][b];
                for(t = i + 4; t < i + 8; t++)
                    for(b = j; b < j + 4; b++)
                        B[b][t] = A[t][b];
            }
        }
}

char transpose64_4block_desc[] = "Transpose64 4block and subblock";
void transpose64_4block(int M, int N, int A[N][M], int B[M][N]){
    for(int i = 0; i < N; i += 4 )
        for(int j = 0; j < M; j += 4){
            for(int b = 0; b < 4; b++){
                int a1=A[i+b][j], a2=A[i+b][j+1], a3=A[i+b][j+2], a4=A[i+b][j+3];
                B[j][i+b] = a1;
                B[j+1][i+b] = a2;
                B[j+2][i+b] = a3;
                B[j+3][i+b] = a4;
            }
        }
}

char transpose64_8block_desc[] = "Transpose64 8block and subblock";
void transpose64_8block(int M, int N, int A[N][M], int B[M][N]){
    for(int i = 0; i < N; i += 8)
        for(int j = 0; j < M; j += 8){
            int a1, a2, a3, a4, a5, a6, a7, a8, t, b;
            //B左上矩阵
            for(t = 0; t < 4; t++){
                a1=A[i+t][j], a2=A[i+t][j+1], a3=A[i+t][j+2], a4=A[i+t][j+3];
                a5=A[i+t][j+4], a6=A[i+t][j+5], a7=A[i+t][j+6], a8=A[i+t][j+7];
                B[j][i+t] = a1;
                B[j+1][i+t] = a2;
                B[j+2][i+t] = a3;
                B[j+3][i+t] = a4;
                //为防止冲突，剩下四个先放在右边4×4的子矩阵
                B[j][i+t+4] = a5;
                B[j+1][i+t+4] = a6;
                B[j+2][i+t+4] = a7;
                B[j+3][i+t+4] = a8;
            }
            //B左下矩阵，将右边的子矩阵放到下面
            for(t = 0; t < 4; t++){
                a1 = B[j+t][i+4];
                a2 = B[j+t][i+5];
                a3 = B[j+t][i+6];
                a4 = B[j+t][i+7];
                B[j+4+t][i] = a1;
                B[j+4+t][i+1] = a2;
                B[j+4+t][i+2] = a3;
                B[j+4+t][i+3]= a4;
            }
            //处理B右上、右下两个子矩阵，完全冲突，只能4×4的块来搞。
            //也就是处理A左下和右下两个子矩阵。
            for(b = 0; b < 4; b++){
                    a1=A[i+4+b][j], a2=A[i+4+b][j+1], a3=A[i+4+b][j+2], a4=A[i+4+b][j+3];
                    B[j][i+4+b] = a1;
                    B[j+1][i+4+b] = a2;
                    B[j+2][i+4+b] = a3;
                    B[j+3][i+4+b] = a4;
                }

             for(b = 0; b < 4; b++){
                    a1=A[i+4+b][j+4], a2=A[i+4+b][j+5], a3=A[i+4+b][j+6], a4=A[i+4+b][j+7];
                    B[j+4][i+4+b] = a1;
                    B[j+5][i+4+b] = a2;
                    B[j+6][i+4+b] = a3;
                    B[j+7][i+4+b] = a4;
                }
        }
}
/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    //registerTransFunction(transpose64_8block, transpose64_8block_desc);

    //registerTransFunction(transpose64_4block, transpose64_4block_desc);

    //registerTransFunction(transpose64_diagonal, transpose64_diagonal_desc);

    //registerTransFunction(transpose64_3part, transpose64_3part_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

