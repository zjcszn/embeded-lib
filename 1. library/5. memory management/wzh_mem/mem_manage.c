/********************************************************************************
* @File name: mem_manage.c
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
********************************************************************************/
#include <stdint.h>
#include <string.h>
#include "mem_manage.h"

#define MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT                8
#define MEM_MANAGE_BITS_PER_BYTE                         8
#define MEM_MANAGE_MEM_STRUCT_SIZE                       Mem_Manage_Align_Up(sizeof(Mem_Node),MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT)
#define MEM_MANAGE_MINUM_MEM_SIZE                        (MEM_MANAGE_MEM_STRUCT_SIZE<<1)
#define MEM_MANAGE_ALLOCA_LABAL                          ((size_t)(1<<(sizeof(size_t)*MEM_MANAGE_BITS_PER_BYTE-1)))
#define MEM_MANAGE_MINUM_NODE_SIZE                       (MEM_MANAGE_MEM_STRUCT_SIZE+MEM_MANAGE_MINUM_MEM_SIZE)
#define MEM_MANAGE_MEM_ROOT_SIZE                         Mem_Manage_Align_Up(sizeof(Mem_Root),MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT)

static __inline size_t Mem_Manage_Align_Down(size_t data, size_t align_byte) {
        return data&~(align_byte - 1);
}

static __inline size_t Mem_Manage_Align_Up(size_t data, size_t align_byte) {
        return (data + align_byte - 1)&~(align_byte - 1);
}

static __inline Mem_Node* Mem_Manage_Addr_To_Mem(const void* addr) {
        return (Mem_Node*)((const uint8_t*)addr - MEM_MANAGE_MEM_STRUCT_SIZE);
}

static __inline void* Mem_Manage_Mem_To_Addr(const Mem_Node* mem_node) {
        return (void*)((const uint8_t*)mem_node + MEM_MANAGE_MEM_STRUCT_SIZE);
}

//将内存节点插入空闲列表中
static __inline void Mem_Insert_Node_To_FreeList(Mem_Root* pRoot, Mem_Node* pNode) {
        Mem_Node* pPriv_Node;
        Mem_Node* pNext_Node;
        //寻找地址与pNode相近的节点
        for (pPriv_Node = pRoot->pStart; pPriv_Node->next_node < pNode; pPriv_Node = pPriv_Node->next_node);
        pNext_Node = pPriv_Node->next_node;
        pRoot->remain_size += pNode->mem_size;
        //尝试pNode与前一个块进行合并
        if ((uint8_t*)Mem_Manage_Mem_To_Addr(pPriv_Node) + pPriv_Node->mem_size == (uint8_t*)pNode) {
                if (pPriv_Node != pRoot->pStart) {//不是Start块的话可以合并
                        pPriv_Node->mem_size += MEM_MANAGE_MEM_STRUCT_SIZE + pNode->mem_size;
                        pRoot->remain_size += MEM_MANAGE_MEM_STRUCT_SIZE;
                        pNode = pPriv_Node;
                }
                else {//后面如果是Start块不进行合并，以免浪费内存
                        pRoot->pStart->next_node = pNode;
                }
        }
        else {//不能合并时直接插入到空闲单链表中
                pPriv_Node->next_node = pNode;
        }
        //尝试后面一个块与pNode进行合并
        if ((uint8_t*)Mem_Manage_Mem_To_Addr(pNode) + pNode->mem_size == (uint8_t*)pNext_Node) {
                if (pNext_Node != pRoot->pEnd) {//不是end块的话可以进行块合并
                        pNode->mem_size += MEM_MANAGE_MEM_STRUCT_SIZE + pNext_Node->mem_size;
                        pRoot->remain_size += MEM_MANAGE_MEM_STRUCT_SIZE;
                        pNode->next_node = pNext_Node->next_node;
                }
                else {//后面如果是end块不进行合并，以免浪费内存
                        pNode->next_node = pRoot->pEnd;
                }
        }
        else {//不能合并时直接插入到空闲单链表中
                pNode->next_node = pNext_Node;
        }
}

static __inline void Mem_Settle(Mem_Root* pRoot){
        Mem_Node* pNode=pRoot->pStart->next_node;
        while(pNode->next_node!=pRoot->pEnd){
                if((uint8_t*)Mem_Manage_Mem_To_Addr(pNode)+pNode->mem_size==(uint8_t*)pNode->next_node){
                        pNode->mem_size += MEM_MANAGE_MEM_STRUCT_SIZE+pNode->next_node->mem_size;
                        pRoot->remain_size += MEM_MANAGE_MEM_STRUCT_SIZE;
                        pNode->next_node=pNode->next_node->next_node;
                }
                else
                        pNode=pNode->next_node;
        }
}

//获取管理内存的状态
//pRoot:句柄指针
//pState:状态信息结构体指针
//return:无返回值
void Mem_Manage_Get_State(Mem_Root* pRoot,Mem_State* pState) {
        
        if(pRoot->err_flag!=MEM_NO_ERR){
                pState->free_node_num=0;
                pState->max_node_size=0;
                pState->min_node_size=0;
                return;
        }
        
        if(pRoot->pStart==NULL||pRoot->pEnd==NULL){
                pRoot->err_flag=MEM_NO_INIT;
                pState->free_node_num=0;
                pState->max_node_size=0;
                pState->min_node_size=0;
                return;
        }
        pState->max_node_size = pRoot->pStart->next_node->mem_size;
        pState->min_node_size = pRoot->pStart->next_node->mem_size;
        pState->free_node_num = 0;
        for (Mem_Node* pNode = pRoot->pStart->next_node; pNode->next_node != NULL; pNode = pNode->next_node) {
                pState->free_node_num ++;
                if (pNode->mem_size > pState->max_node_size)
                        pState->max_node_size = pNode->mem_size;
                if (pNode->mem_size < pState->min_node_size)
                        pState->min_node_size = pNode->mem_size;
        }
}

//与C库函数aligned_alloc作用一致
//pRoot:句柄指针
//align_size:期望分配的内存几字节对齐（8、16、32...)
//want_size:期望分配内存大小
//return:        NULL 分配失败（内存不足或者错误标记不为MEM_NO_ERR）；
//                        其他值 分配成功
void* Mem_Manage_Aligned_Alloc(Mem_Root* pRoot,size_t align_size, size_t want_size) {
        void* pReturn = NULL;
        Mem_Node* pPriv_Node,*pNow_Node;

        if(pRoot->err_flag!=MEM_NO_ERR){
                return NULL;
        }
        
        if(pRoot->pStart==NULL||pRoot->pEnd==NULL){
                pRoot->err_flag=MEM_NO_INIT;
                return NULL;
        }
        
        if (want_size == 0) {
                return NULL;
        }

        if ((want_size&MEM_MANAGE_ALLOCA_LABAL) != 0) {//内存过大
                return NULL;
        }

        if (align_size&(align_size - 1)) {//内存对齐输入非法值
                return NULL;
        }
        
        if (want_size < MEM_MANAGE_MINUM_MEM_SIZE)
                want_size = MEM_MANAGE_MINUM_MEM_SIZE;
        if (align_size < MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT)
                align_size = MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT;
        //确保分配的单元都是MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT的整数倍
        want_size = Mem_Manage_Align_Up(want_size, MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);

        pPriv_Node = pRoot->pStart;
        pNow_Node = pRoot->pStart->next_node;
        
        while (pNow_Node->next_node != NULL) {
                if (pNow_Node->mem_size >= want_size+ MEM_MANAGE_MEM_STRUCT_SIZE) {
                        size_t use_align_size;
                        size_t new_size;
                        pReturn = (void*)Mem_Manage_Align_Up((size_t)Mem_Manage_Mem_To_Addr(pNow_Node), align_size);//计算出对齐的地址
                        use_align_size = (uint8_t*)pReturn-(uint8_t*)Mem_Manage_Mem_To_Addr(pNow_Node);//计算对齐所消耗的内存
                        if (use_align_size != 0) {//内存不对齐
                                if (use_align_size < MEM_MANAGE_MINUM_NODE_SIZE) {//不对齐的值过小
                                        pReturn = (void*)Mem_Manage_Align_Up(\
                                                (size_t)Mem_Manage_Mem_To_Addr(pNow_Node)+ MEM_MANAGE_MINUM_NODE_SIZE, align_size);
                                        use_align_size = (uint8_t*)pReturn - (uint8_t*)Mem_Manage_Mem_To_Addr(pNow_Node);
                                }
                                if (use_align_size <= pNow_Node->mem_size) {
                                        new_size = pNow_Node->mem_size - use_align_size;//计算去除对齐消耗的内存剩下的内存大小
                                        if (new_size >= want_size) {//满足条件，可以进行分配
                                                Mem_Node* pNew_Node = Mem_Manage_Addr_To_Mem(pReturn);
                                                pNow_Node->mem_size -= new_size + MEM_MANAGE_MEM_STRUCT_SIZE;//分裂节点
                                                pRoot->remain_size -= new_size + MEM_MANAGE_MEM_STRUCT_SIZE;
                                                pNew_Node->mem_size = new_size;//新节点本来也不在空闲链表中，不用从空闲链表中排出
                                                pNew_Node->next_node = (Mem_Node*)MEM_NO_ERR;
                                                pNow_Node = pNew_Node;
                                                break;
                                        }
                                }
                        }
                        else {//内存直接就是对齐的
                                pPriv_Node->next_node = pNow_Node->next_node;//排出空闲链表
                                pNow_Node->next_node = (Mem_Node*)MEM_NO_ERR;
                                pRoot->remain_size -= pNow_Node->mem_size;
                                break;
                        }
                }
                pPriv_Node = pNow_Node;
                pNow_Node = pNow_Node->next_node;
        }

        if (pNow_Node->next_node == NULL){//分配失败
                if(pNow_Node!=pRoot->pEnd){
                        pRoot->err_flag=MEM_OVER_WRITE;
                }
                return NULL;
        }
        pNow_Node->next_node = NULL;
        if (pNow_Node->mem_size >= MEM_MANAGE_MINUM_NODE_SIZE + want_size) {//节点内存还有富余
                Mem_Node* pNew_Node =(Mem_Node*)((uint8_t*)Mem_Manage_Mem_To_Addr(pNow_Node) + want_size);//计算将要移入空闲链表的节点地址
                pNew_Node->mem_size = pNow_Node->mem_size - want_size - MEM_MANAGE_MEM_STRUCT_SIZE;
                pNew_Node->next_node = NULL;
                pNow_Node->mem_size = want_size;
                Mem_Insert_Node_To_FreeList(pRoot, pNew_Node);
        }
        pNow_Node->mem_size |= MEM_MANAGE_ALLOCA_LABAL;//标记内存已分配
        return pReturn;
}

//与C库函数malloc作用相同
//pRoot:句柄指针
//want_size:期望分配内存大小
//return:        NULL 分配失败（内存不足或者错误标记不为MEM_NO_ERR）；
//                        其他值 分配成功
void* Mem_Manage_Malloc(Mem_Root* pRoot, size_t want_size) {
        return Mem_Manage_Aligned_Alloc(pRoot, MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT, want_size);
}

//与C库函数realloc作用相同
//pRoot:句柄指针
//src_addr:源地址指针
//want_size:期望分配内存大小
//return:        NULL 分配失败（内存不足或者句柄错误标记不为MEM_NO_ERR）；
//                        其他值 分配成功
void* Mem_Manage_Realloc(Mem_Root* pRoot, void* src_addr, size_t want_size) {
        void* pReturn = NULL;
        Mem_Node* pNext_Node,*pPriv_Node;
        Mem_Node* pSrc_Node;
        
        if(pRoot->err_flag!=MEM_NO_ERR){
                return NULL;
        }
        
        if(pRoot->pStart==NULL||pRoot->pEnd==NULL){
                pRoot->err_flag=MEM_NO_INIT;
                return NULL;
        }
        
        if (src_addr == NULL) {
                return Mem_Manage_Aligned_Alloc(pRoot, MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT, want_size);
        }
        if (want_size == 0) {
                Mem_Manage_Free(pRoot, src_addr);
                return NULL;
        }

        if ((want_size&MEM_MANAGE_ALLOCA_LABAL) != 0){
                return NULL;
        }

        pSrc_Node = Mem_Manage_Addr_To_Mem(src_addr);

        if ((pSrc_Node->mem_size&MEM_MANAGE_ALLOCA_LABAL) == 0) {//源地址未被分配，调用错误
                pRoot->err_flag=MEM_OVER_WRITE;
                return NULL;
        }

        pSrc_Node->mem_size &= ~MEM_MANAGE_ALLOCA_LABAL;//清除分配标记
        if (pSrc_Node->mem_size >= want_size) {//块预留地址足够大
                pSrc_Node->mem_size |= MEM_MANAGE_ALLOCA_LABAL;//恢复分配标记
                pReturn = src_addr;
                return pReturn;
        }
        //开始在空闲列表中寻找与本块相近的块
        for (pPriv_Node = pRoot->pStart; pPriv_Node->next_node <pSrc_Node; pPriv_Node = pPriv_Node->next_node);
        pNext_Node = pPriv_Node->next_node;

        if (pNext_Node != pRoot->pEnd && \
                ((uint8_t*)src_addr + pSrc_Node->mem_size == (uint8_t*)pNext_Node) && \
                (pSrc_Node->mem_size + pNext_Node->mem_size + MEM_MANAGE_MEM_STRUCT_SIZE >= want_size)) {
                //满足下一节点非end，内存连续，内存剩余足够
                pReturn = src_addr;
                pPriv_Node->next_node = pNext_Node->next_node;//排出空闲列表
                pRoot->remain_size -= pNext_Node->mem_size;
                pSrc_Node->mem_size += MEM_MANAGE_MEM_STRUCT_SIZE + pNext_Node->mem_size;
                want_size = Mem_Manage_Align_Up(want_size, MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
                if (pSrc_Node->mem_size >= MEM_MANAGE_MINUM_NODE_SIZE+ want_size) {//去除分配的剩余空间足够开辟新块
                        Mem_Node* pNew_Node = (Mem_Node*)((uint8_t*)Mem_Manage_Mem_To_Addr(pSrc_Node) + want_size);
                        pNew_Node->next_node = NULL;
                        pNew_Node->mem_size = pSrc_Node->mem_size - want_size - MEM_MANAGE_MEM_STRUCT_SIZE;
                        pSrc_Node->mem_size = want_size;
                        Mem_Insert_Node_To_FreeList(pRoot, pNew_Node);
                }
                pSrc_Node->mem_size |= MEM_MANAGE_ALLOCA_LABAL;//恢复分配标记
        }
        else {
                pReturn = Mem_Manage_Aligned_Alloc(pRoot, MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT, want_size);
                if (pReturn == NULL){
                        pSrc_Node->mem_size |= MEM_MANAGE_ALLOCA_LABAL;//恢复分配标记
                        return NULL;
                }
                memcpy(pReturn, src_addr, pSrc_Node->mem_size);
                pSrc_Node->mem_size |= MEM_MANAGE_ALLOCA_LABAL;//恢复分配标记
                Mem_Manage_Free(pRoot, src_addr);
        }
        return pReturn;
}

//与C库函数free作用一致
//pRoot:句柄指针
//addr:释放内存的首地址
//return:无返回值
void Mem_Manage_Free(Mem_Root* pRoot,void* addr) {
        Mem_Node* pFree_Node;
        
        if(pRoot->err_flag!=MEM_NO_ERR){
                return;
        }
        
        if(pRoot->pStart==NULL||pRoot->pEnd==NULL){
                pRoot->err_flag=MEM_NO_INIT;
                return;
        }
        
        if (addr == NULL) {
                return;
        }
        pFree_Node = Mem_Manage_Addr_To_Mem(addr);

        if ((pFree_Node->mem_size&MEM_MANAGE_ALLOCA_LABAL) == 0) {//释放错误，没有标记
                pRoot->err_flag=MEM_OVER_WRITE;
                return;
        }

        if (pFree_Node->next_node != NULL) {//释放错误
                pRoot->err_flag=MEM_OVER_WRITE;
                return;
        }
        pFree_Node->mem_size &= ~MEM_MANAGE_ALLOCA_LABAL;//清除分配标记
        Mem_Insert_Node_To_FreeList(pRoot, pFree_Node);//插入到空闲链表中
}

//获取句柄管理的内存区总容量
//pRoot:句柄指针
//return:内存区总容量（单位：byte）
size_t Mem_Manage_Get_Total_Size(const Mem_Root* pRoot){
        return pRoot->total_size;
}

//获取句柄管理的内存区剩余容量
//pRoot:句柄指针
//return:内存区剩余容量（单位：byte）
size_t Mem_Manage_Get_Remain_Size(const Mem_Root* pRoot){
        return pRoot->remain_size;
}

//获取句柄管理的内存区错误标记
//pRoot:句柄指针
//return:错误标记
Mem_Err_Type Mem_Manage_Get_Errflag(const Mem_Root* pRoot){
        return pRoot->err_flag;
}

//内存管理句柄初始化
//pRoot:句柄指针
//pRigon:内存区结构体指针
//return:        true 初始化成功;
//                        false 初始化失败
bool Mem_Manage_Init(Mem_Root* pRoot,const Mem_Region* pRegion) {
        Mem_Node* align_addr;
        size_t align_size;
        Mem_Node* pPriv_node=NULL;

        pRoot->total_size = 0;
        pRoot->pEnd = NULL;
        pRoot->pStart = NULL;
        pRoot->err_flag = MEM_NO_INIT;
        pRoot->remain_size = 0;
        for (; pRegion->addr != NULL; pRegion++) {
                align_addr = (Mem_Node*)Mem_Manage_Align_Up((size_t)pRegion->addr, MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);//计算内存块对齐后的地址
                if ((uint8_t*)align_addr > pRegion->mem_size+ (uint8_t*)pRegion->addr)//对齐消耗的内存超过内存区
                        continue;
                align_size = pRegion->mem_size - ((uint8_t*)align_addr - (uint8_t*)pRegion->addr);//计算对齐后剩下的内存大小
                if (align_size < MEM_MANAGE_MINUM_MEM_SIZE+ MEM_MANAGE_MEM_STRUCT_SIZE)//对齐剩下的内存太小
                        continue;
                align_size -= MEM_MANAGE_MEM_STRUCT_SIZE;//求除去掉表头后内存块的大小
                align_addr->mem_size = align_size;
                align_addr->next_node = NULL;
                if (pRoot->pStart == NULL) {//如果是初始化
                        pRoot->pStart = align_addr;//将当前内存块地址记为start
                        if (align_size >= MEM_MANAGE_MINUM_MEM_SIZE+ MEM_MANAGE_MEM_STRUCT_SIZE) {//若剩下的块足够大
                                align_size -= MEM_MANAGE_MEM_STRUCT_SIZE;//去掉下一个块的表头剩下的内存大小
                                align_addr = (Mem_Node*)((uint8_t*)pRoot->pStart + MEM_MANAGE_MEM_STRUCT_SIZE);//下一个块的表头地址
                                align_addr->mem_size = align_size;
                                align_addr->next_node = NULL;
                                pRoot->pStart->mem_size = 0;
                                pRoot->pStart->next_node = align_addr;
                                pRoot->total_size = align_addr->mem_size;
                        }
                        else {//内存太小了，将当前内存块地址记为start
                                pRoot->total_size = 0;
                                pRoot->pStart->mem_size = 0;
                        }
                }
                else {
                        if (pPriv_node == NULL) {
                                pRoot->err_flag = MEM_NO_INIT;
                                return false;
                        }
                        pPriv_node->next_node = align_addr;//更新上一节点的next_node
                        pRoot->total_size += align_size;
                }
                pPriv_node = align_addr;
        }
        if (pPriv_node == NULL) {
                pRoot->err_flag = MEM_NO_INIT;
                return false;
        }
        //此时，pPriv_node为最后一个块，接下来在块尾放置表尾end
        //求出放置end块的地址,end块仅是方便遍历使用，因此尽量小，分配为MEM_MANAGE_MEM_STRUCT_SIZE
        align_addr = (Mem_Node*)Mem_Manage_Align_Down(\
                (size_t)Mem_Manage_Mem_To_Addr(pPriv_node) + pPriv_node->mem_size - MEM_MANAGE_MEM_STRUCT_SIZE, MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        align_size = (uint8_t*)align_addr-(uint8_t*)Mem_Manage_Mem_To_Addr(pPriv_node);//求出分配出end块后，前一个块剩余大小
        if (align_size >= MEM_MANAGE_MINUM_MEM_SIZE) {//若剩下的块足够大
                pRoot->total_size -= pPriv_node->mem_size - align_size;//去掉分配end块消耗的内存
                pRoot->pEnd = align_addr;                        //更新表尾的地址
                pPriv_node->next_node = align_addr;
                pPriv_node->mem_size = align_size;
                align_addr->next_node = NULL;
                align_addr->mem_size = 0;//end块不参与内存分配，因此直接为0就可以
        }
        else {//最后一个块太小了，直接作为end块
                pRoot->pEnd = pPriv_node;
                pRoot->total_size -= pPriv_node->mem_size;
        }
        if(pRoot->pStart==NULL||pRoot->pEnd==NULL){
                pRoot->err_flag=MEM_NO_INIT;
                return false;
        }
        Mem_Settle(pRoot);
        pRoot->err_flag=MEM_NO_ERR;
        pRoot->remain_size=pRoot->total_size;
        return true;
}

//内存池初始化
//mem_addr:内存池首地址
//mem_size:内存池大小
//return:        true 初始化成功;
//                        false 初始化失败
bool Mem_Manage_Pool_Init(void* mem_addr,size_t mem_size){
        void* paddr=(uint8_t*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT)+MEM_MANAGE_MEM_ROOT_SIZE;
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        size_t align_size=(uint8_t*)paddr-(uint8_t*)mem_addr;
        Mem_Region buf_region[]={
                0,0,
                NULL,0
        };
        if(mem_size<align_size)
                return 0;
        mem_size-=align_size;
        if(mem_size<2*MEM_MANAGE_MEM_STRUCT_SIZE+MEM_MANAGE_MINUM_NODE_SIZE)
                return 0;
        buf_region[0].addr=paddr;
        buf_region[0].mem_size=mem_size;
        return Mem_Manage_Init(root_addr,buf_region);
}

//与C库函数malloc作用相同
//mem_addr:内存池首地址
//want_size:期望分配内存大小
//return:        NULL 分配失败（内存不足或者错误标记不为MEM_NO_ERR）；
//                        其他值 分配成功
void* Mem_Manage_Pool_Malloc(void* mem_addr,size_t want_size){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        return Mem_Manage_Malloc(root_addr,want_size);
}

//与C库函数realloc作用相同
//mem_addr:内存池首地址
//src_addr:源地址指针
//want_size:期望分配内存大小
//return:        NULL 分配失败（内存不足或者错误标记不为MEM_NO_ERR）；
//                        其他值 分配成功
void* Mem_Manage_Pool_Realloc(void* mem_addr,void* src_addr,size_t want_size){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        return Mem_Manage_Realloc(root_addr,src_addr,want_size);
}

//与C库函数aligned_alloc作用一致
//mem_addr:内存池首地址
//align_size:期望分配的内存几字节对齐（8、16、32...)
//want_size:期望分配内存大小
//return:        NULL 分配失败（内存不足或者句柄错误标记不为MEM_NO_ERR）；
//                        其他值 分配成功
void* Mem_Manage_Pool_Aligned_Alloc(void* mem_addr,size_t align_byte,size_t want_size){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        return Mem_Manage_Aligned_Alloc(root_addr,align_byte,want_size);
}

//与C库函数free作用一致
//mem_addr:内存池首地址
//free_addr:释放内存的首地址
//return:无返回值
void Mem_Manage_Pool_Free(void* mem_addr,void* free_addr){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        Mem_Manage_Free(root_addr,free_addr);
}

//获取内存池的状态
//mem_addr:内存池首地址
//pState:状态信息结构体指针
//return:无返回值
void Mem_Manage_Pool_Get_State(void* mem_addr,Mem_State* pState){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        Mem_Manage_Get_State(root_addr,pState);
}

//获取内存池总容量
//mem_addr:内存池首地址
//return:内存区总容量（单位：byte）
size_t Mem_Manage_Pool_Get_Total_Size(const void* mem_addr){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        return Mem_Manage_Get_Total_Size(root_addr);
}

//获取内存池剩余容量
//mem_addr:内存池首地址
//return:内存区剩余容量（单位：byte）
size_t Mem_Manage_Pool_Get_Remain_Size(const void* mem_addr){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        return Mem_Manage_Get_Remain_Size(root_addr);
}

//获取内存池错误标记
//mem_addr:内存池首地址
//return:错误标记
Mem_Err_Type Mem_Manage_Pool_Get_Errflag(const void* mem_addr){
        Mem_Root* root_addr=(Mem_Root*)Mem_Manage_Align_Up((size_t)mem_addr,MEM_MANAGE_ALIGNMENT_BYTE_DEFAULT);
        return Mem_Manage_Get_Errflag(root_addr);
}
