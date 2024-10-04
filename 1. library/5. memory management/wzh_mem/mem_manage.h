/********************************************************************************
* @File name: mem_manage.h
* @Author: wzh
* @Version: 1.2
* @Date: 2021-9-3
* @Description: 内存管理算法，带有内存碎片合并算法，支持malloc、align_alloc、
*                                realloc、free等常见的内存管理函数，支持多块内存合并管理
*更新记录：
*               v1.0 2021-8-13  添加Mem_Manage_Heap_Init、Mem_Manage_Malloc、Mem_Manage_Realloc
                                                Mem_Manage_Aligned_Alloc函数
                v1.1 2021-8-14  添加Mem_Manage_Get_State函数
                v1.2 2021-9-3   更改Mem_Root结构体成员；更改Mem_State结构体成员；
                                                添加枚举类型Mem_Err_Type；将函数Mem_Manage_Heap_Init重命名为
                                                Mem_Manage_Init；修改Mem_Manage_Init函数声明；添加函数Mem_Manage_Get_Total_Size、
                                                Mem_Manage_Get_Remain_Size、Mem_Manage_Get_Errflag、Mem_Manage_Pool_Init、
                                                Mem_Manage_Pool_Malloc、Mem_Manage_Pool_Realloc、Mem_Manage_Pool_Aligned_Alloc、
                                                Mem_Manage_Pool_Free、Mem_Manage_Pool_Get_State、Mem_Manage_Pool_Get_Total_Size、
                                                Mem_Manage_Pool_Get_Remain_Size、Mem_Manage_Pool_Get_Errflag；
* @Todo        具体使用方法如下
*                 1、使用Mem_Manage_Init(Mem_Root* pRoot, const Mem_Region* pRigon)初始化
*                        内存区，pRoot为句柄，pRigon描述了内存区个数以及内个内存区起始地址和大小
*                        pRigon的格式如下
*                        const Mem_Region pRigon[]=
*                        {
*                                (void*)(0x20000000),512*1024,
*                                (void*)(0x80000000),256*1024,
*                                ....
*                                NULL,0
*                        }
*                        注意地址必需由低到高排列，同时使用NULL，0标识结尾，内存区容量不要太小，至少大于64个字节
*                 2、使用Mem_Manage_Malloc、Mem_Manage_Realloc、Mem_Manage_Aligned_Alloc进行内存
*                        分配，其中Mem_Manage_Malloc、Mem_Manage_Realloc默认均为8字节对齐，可修改
*                        .c文件中的宏定义修改，Mem_Manage_Aligned_Alloc可以指定地址对齐，但对齐的参数
*                        有限制，align_size需要为2的整数次幂，否则会直接返回NULL。
*                 3、内存使用完毕后使用Mem_Manage_Free进行内存释放
*                 4、可通过Mem_Manage_Get_State查看空闲内存状态，通过Mem_Manage_Get_Total_Size获取总内存，
*                        通过Mem_Manage_Get_Remain_Size获取剩余内存                
*                 5、算法管理的单个内存上限为2GB（32位机）
********************************************************************************/
#ifndef MEM_MANAGE_H_
#define MEM_MANAGE_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        MEM_NO_ERR=0X1234,            //无错误
        MEM_NO_INIT=0,                //内存区未初始化
        MEM_OVER_WRITE=1              //内存区节点信息位置数据损坏
}Mem_Err_Type;

typedef struct Mem_Region {
        void*   addr;                 //内存区起始地址
        size_t  mem_size;             //内存大小
}Mem_Region;

typedef struct Mem_Node {
        struct Mem_Node* next_node;
        size_t mem_size;
}Mem_Node;

typedef struct Mem_Root {
        Mem_Node* pStart;
        Mem_Node* pEnd;
        size_t total_size;            //总内存
        size_t remain_size;           //剩余内存
        Mem_Err_Type err_flag;        //错误标记
}Mem_Root;

typedef struct Mem_State {
        size_t free_node_num;         //空闲节点个数
        size_t max_node_size;         //最大节点内存
        size_t min_node_size;         //最小节点内存
}Mem_State;


bool    Mem_Manage_Init(Mem_Root* pRoot, const Mem_Region* pRigon);
void*   Mem_Manage_Malloc(Mem_Root* pRoot, size_t want_size);
void*   Mem_Manage_Realloc(Mem_Root* pRoot, void* src_addr, size_t want_size);
void*   Mem_Manage_Aligned_Alloc(Mem_Root* pRoot, size_t align_size, size_t want_size);
void    Mem_Manage_Free(Mem_Root* pRoot, void* addr);
void    Mem_Manage_Get_State(Mem_Root* pRoot, Mem_State* pState);
size_t  Mem_Manage_Get_Total_Size(const Mem_Root* pRoot);
size_t  Mem_Manage_Get_Remain_Size(const Mem_Root* pRoot);
Mem_Err_Type Mem_Manage_Get_Errflag(const Mem_Root* pRoot);

bool    Mem_Manage_Pool_Init(void* mem_addr,size_t mem_size);
void*   Mem_Manage_Pool_Malloc(void* mem_addr,size_t want_size);
void*   Mem_Manage_Pool_Realloc(void* mem_addr,void* src_addr,size_t want_size);
void*   Mem_Manage_Pool_Aligned_Alloc(void* mem_addr,size_t align_byte,size_t want_size);
void    Mem_Manage_Pool_Free(void* mem_addr,void* free_addr);
void    Mem_Manage_Pool_Get_State(void* mem_addr,Mem_State* pState);
size_t  Mem_Manage_Pool_Get_Total_Size(const void* mem_addr);
size_t  Mem_Manage_Pool_Get_Remain_Size(const void* mem_addr);
Mem_Err_Type Mem_Manage_Pool_Get_Errflag(const void* mem_addr);

#ifdef __cplusplus
}
#endif

#endif

