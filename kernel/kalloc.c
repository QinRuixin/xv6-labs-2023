// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define REF_IDX(pa) ((((uint64)pa) - KERNBASE) / PGSIZE)

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  ushort refcnt[REF_IDX(PHYSTOP)];
  uint64 usedmem;
  uint64 freemem;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  for(int i = 0; i < REF_IDX(PHYSTOP); ++i){
    kmem.refcnt[i] = 1;
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
  kmem.usedmem = 0;
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  kmem.refcnt[REF_IDX(pa)] -= 1;
  if(kmem.refcnt[REF_IDX(pa)] > 0){
    release(&kmem.lock);
    return;
  }
  release(&kmem.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.freemem += PGSIZE;
  kmem.usedmem -= PGSIZE;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.refcnt[REF_IDX(r)] = 1;
    kmem.freemem -= PGSIZE;
    kmem.usedmem += PGSIZE;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void kincref(void *pa){
  acquire(&kmem.lock);
  kmem.refcnt[REF_IDX(pa)] += 1;
  release(&kmem.lock);
  return;
}

uint64
kfreemem(void)
{
  uint64 free = 0;
  acquire(&kmem.lock);
  free = kmem.freemem;
  release(&kmem.lock);
  // struct run *r = kmem.freelist;
  // while (r){
  //   r = r->next;
  //   free += PGSIZE;
  // }
  return free;
}

uint64
kusedmem(void)
{
  uint64 used = 0;
  acquire(&kmem.lock);
  used = kmem.usedmem;
  release(&kmem.lock);
  return used;
}
