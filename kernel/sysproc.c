#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
// pte_t *
// walk(pagetable_t pagetable, uint64 va, int alloc)
// {
//   if(va >= MAXVA)
//     panic("walk");

//   for(int level = 2; level > 0; level--) {
//     pte_t *pte = &pagetable[PX(level, va)];
//     if(*pte & PTE_V) {
//       pagetable = (pagetable_t)PTE2PA(*pte);
//     } else {
//       if(!alloc || (pagetable = (pde_t*)kalloc()) == 0)
//         return 0;
//       memset(pagetable, 0, PGSIZE);
//       *pte = PA2PTE(pagetable) | PTE_V;
//     }
//   }
//   return &pagetable[PX(0, va)];
// }
pte_t *
walk(pagetable_t pagetable, uint64 va, int alloc);
void
vmprint(pagetable_t pagetable);
int
sys_pgaccess(void)
{
 uint64 buffer,bitmask;
 uint64 temp_mask=0;
 int num;
 struct proc* p=myproc();
 vmprint(p->pagetable);
 pte_t * temp_pte_t;
 argaddr(0,&buffer);
 argint(1,&num);
 argaddr(2,&bitmask);
  if(num>32)
    return -1;
  for(int i=0;i!=num;i++){
    if((temp_pte_t=walk(p->pagetable,buffer+PGSIZE*i,0))!=0){
        if((*temp_pte_t)&PTE_A)
          temp_mask|=(1<<i);
         *temp_pte_t=(*temp_pte_t)&(~PTE_A);
    }
  }
  copyout(p->pagetable,bitmask,(char *)&temp_mask,4);
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
