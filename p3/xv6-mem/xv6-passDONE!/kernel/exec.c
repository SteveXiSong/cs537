#include "types.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  cprintf("-(%d)exec...\n",proc->pid);
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;

  if((ip = namei(path)) == 0)
    return -1;
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) < sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  // sx: passed Part 1
  sz = PGSIZE;//PGSIZW = 4K
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.va + ph.memsz)) == 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.va, ip, ph.offset, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  ip = 0;
  //no need to modify above/////////////////////////
  // Allocate a one-page stack at the next page boundary
  sz = PGROUNDUP(sz);
  /*
  if((sz = allocuvm(pgdir, sz, sz + PGSIZE)) == 0)
    goto bad;
  sp = sz;
  */

  // Push argument strings, prepare rest of stack in ustack.
  // sx:tst1/////////////////
  int sz_stack= 0;
  if((sz_stack = allocuvm(pgdir, USERTOP-PGSIZE, USERTOP))==0)
  	panic("allocuvm sz_stack failed\n");
  sp = sz_stack; 
  
  /////////////////////////
  cprintf("-exec: sp(%d)\n",sp);
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp -= strlen(argv[argc]) + 1;
    sp &= ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(proc->name, last, sizeof(proc->name));

  cprintf("-exec:sz(%d)sp(%d)sz-sp(%d)elf.entry(%d)\n",sz,sp,USERTOP-sp,elf.entry);
  // Commit to the user image.
  oldpgdir = proc->pgdir;
  proc->pgdir = pgdir;
  proc->sz = sz_stack;//current stack bottom
  proc->sz_stack = sz_stack-PGSIZE;//current stack top
  proc->sz_heap = sz;
  proc->tf->eip = elf.entry;  // main
  proc->tf->esp = sp;
  switchuvm(proc);
  freevm(oldpgdir);

  cprintf("-exec end***\n");
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip)
    iunlockput(ip);
  return -1;
}
