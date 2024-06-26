#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "ps.h"

extern struct proc proc[NPROC];

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

//return the trace of the system
uint64
sys_trace(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  struct proc *p = myproc();
  p->trace_mask = n;
  return 0;  // not reached
}

uint64
sys_pinfo(void) {
  uint64 addr;
  if(argaddr(0, &addr) < 0)
    return -1;
  struct proc *p;
  struct proc *p2 = myproc();
  struct psinfo *psi = kalloc();
  int ps_proc_num = 0;
  for(p = proc; p < &proc[NPROC]; p++) {
    if(ps_proc_num > 64) //MAX_PS_PROC
      exit(1);
    acquire(&p->lock);
    if(p->state != UNUSED){
      psi->active[ps_proc_num] = 1;
      copyout(p2->pagetable, addr + ps_proc_num*4, (char *)psi+ps_proc_num*4, 4);

      psi->pid[ps_proc_num] = p->pid;
      copyout(p2->pagetable, addr + 256 + ps_proc_num*4, (char *)psi+ 256 + ps_proc_num*4, 4);

      psi->states[ps_proc_num] = p->state;
      copyout(p2->pagetable, addr + 512 +  ps_proc_num*4, (char *)psi+ 512 +  ps_proc_num*4, 4);

      psi->num_used_pages[ps_proc_num] = countmapped(p->pagetable);
      copyout(p2->pagetable, addr + 768 + ps_proc_num*4, (char *) psi+768 + ps_proc_num*4, 4);

      strncpy(psi->name[ps_proc_num], p->name, 16);
      copyout(p2->pagetable, addr+1024+ps_proc_num*16, (char *)psi+1024+ps_proc_num*16, 16);
    }
    release(&p->lock);
    ps_proc_num++;
  }

  //copyout(p2->pagetable, addr, (char *)&psi, sizeof(struct psinfo));
  kfree(psi);
  return 0;
}
