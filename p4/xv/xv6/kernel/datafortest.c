Binary file bootblock.out matches
bootmain.c:    readseg(va, ph->filesz, ph->offset);
bootmain.c:    if(ph->memsz > ph->filesz)
bootmain.c:      stosb(va + ph->filesz, 0, ph->memsz - ph->filesz);
Binary file bootmain.o matches
Binary file console.o matches
elf.h:  uint filesz;
elf.h:  uint memsz;
exec.c:  uint argc, sz, sp, ustack[3+MAXARG+1];
exec.c:  sz = 0;
exec.c:    if(ph.memsz < ph.filesz)
exec.c:    if((sz = allocuvm(pgdir, sz, ph.va + ph.memsz)) == 0)
exec.c:    if(loaduvm(pgdir, (char*)ph.va, ip, ph.offset, ph.filesz) < 0)
exec.c:  sz = PGROUNDUP(sz);
exec.c:  if((sz = allocuvm(pgdir, sz, sz + PGSIZE)) == 0)
exec.c:  sp = sz;
exec.c:  proc->sz = sz;
Binary file exec.o matches
Binary file fs.o matches
Binary file ide.o matches
Binary file kernel matches
Binary file main.o matches
mmu.h:#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
Binary file mp.o matches
Binary file pipe.o matches
proc.c:  p->sz = PGSIZE;
proc.c:  uint sz;
proc.c:  sz = proc->parent->sz;//sx
proc.c:  //sz = proc->sz;
proc.c:    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
proc.c:    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
proc.c:  proc->parent->sz = sz;//sx:t size
proc.c:  np->sz = proc->sz;
proc.c:	//if((uint)stack > np->sz) return -1;//if stack is bad
proc.c:  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
proc.c:  np->sz = proc->sz;
proc.h:  uint sz;                     // Size of process memory (bytes)
Binary file proc.o matches
Binary file spinlock.o matches
syscall.c:  if(addr >= p->sz || addr+4 > p->sz)
syscall.c:  if(addr >= p->sz)
syscall.c:  ep = (char*)p->sz;
syscall.c:  if((uint)i >= proc->sz || (uint)i+size > proc->sz)
Binary file syscall.o matches
Binary file sysfile.o matches
sysproc.c:	if((uint)stack+PGSIZE > proc->sz) return -1;
sysproc.c:  addr = proc->parent->sz;//sx:t size
Binary file sysproc.o matches
sz:exec.c:  proc->sz = sz;
sz:proc.c:  sz = proc->parent->sz;//sx
sz:proc.c:  //sz = proc->sz;
sz:proc.c:    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
sz:proc.c:    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
sz:proc.c:  proc->parent->sz = sz;//sx:t size
sz:proc.c:  np->sz = proc->sz;
sz:proc.c:  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
sz:proc.c:  np->sz = proc->sz;
sz:syscall.c:  if((uint)i >= proc->sz || (uint)i+size > proc->sz)
sz:sysproc.c:	if((uint)stack+PGSIZE > proc->sz) return -1;
sz:sysproc.c:  addr = proc->parent->sz;//sx:t size
Binary file trap.o matches
vm.c:// sz must be less than a page.
vm.c:inituvm(pde_t *pgdir, char *init, uint sz)
vm.c:  if(sz >= PGSIZE)
vm.c:  memmove(mem, init, sz);
vm.c:// and the pages from addr to addr+sz must already be mapped.
vm.c:loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
vm.c:  for(i = 0; i < sz; i += PGSIZE){
vm.c:    if(sz - i < PGSIZE)
vm.c:      n = sz - i;
vm.c:// Allocate page tables and physical memory to grow process from oldsz to
vm.c:// newsz, which need not be page aligned.  Returns new size or 0 on error.
vm.c:allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
vm.c:  if(newsz > USERTOP)
vm.c:  if(newsz < oldsz)
vm.c:    return oldsz;
vm.c:  a = PGROUNDUP(oldsz);
vm.c:  for(; a < newsz; a += PGSIZE){
vm.c:      deallocuvm(pgdir, newsz, oldsz);
vm.c:  return newsz;
vm.c:// Deallocate user pages to bring the process size from oldsz to
vm.c:// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
vm.c:// need to be less than oldsz.  oldsz can be larger than the actual
vm.c:deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
vm.c:  if(newsz >= oldsz)
vm.c:    return oldsz;
vm.c:  a = PGROUNDUP(newsz);
vm.c:  for(; a  < oldsz; a += PGSIZE){
vm.c:  return newsz;
vm.c:copyuvm(pde_t *pgdir, uint sz)
vm.c:  for(i = 0; i < sz; i += PGSIZE){
Binary file vm.o matches
