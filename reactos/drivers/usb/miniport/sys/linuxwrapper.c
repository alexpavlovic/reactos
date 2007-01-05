/*
 * USB support based on Linux kernel source
 *
 * 2003-06-21 Georg Acher (georg@acher.org)
 *
 * Concept:
 * 
 * 1) Forget all device interrupts, scheduling, semaphores, threads etc.
 * 1a) Forget all DMA and PCI helper functions
 * 2) Forget usbdevfs, procfs and ioctls
 * 3) Emulate xHCI interrupts and root hub timer by polling
 * 4) Emulate hub kernel thread by polling
 * 5) Emulate synchronous USB-messages (usb_*_msg) with busy waiting
 *
 * To be done:
 * 6) Remove code bloat  
 *
 */

#include "../usb_wrapper.h"

/* internal state */

static struct pci_dev *pci_probe_dev;
extern int (*thread_handler)(void*);
extern void* thread_parm;

struct my_irqs reg_irqs[MAX_IRQS];
int num_irqs;
int need_wakeup;

int my_jiffies;

struct timer_list *main_timer_list[MAX_TIMERS];
PKDEFERRED_ROUTINE timer_dpcs[MAX_TIMERS];
struct dummy_process act_cur={0};
struct dummy_process *my_current;

int (*thread_handler)(void*);
void* thread_parm;

#define MAX_DRVS 8
static struct device_driver *m_drivers[MAX_DRVS];
static int drvs_num=0;
unsigned int LAST_USB_EVENT_TICK;

NTSTATUS init_dma(PUSBMP_DEVICE_EXTENSION pDevExt);

/*------------------------------------------------------------------------*/ 
/* 
 * Helper functions for top-level system
 */
/*------------------------------------------------------------------------*/ 
void init_wrapper(struct pci_dev *probe_dev)
{
	int n;
	for(n=0;n<MAX_TIMERS;n++)
	{
		main_timer_list[n]=NULL;
	}

	// FIXME: Change if more than 5 timers
	timer_dpcs[0] = (PKDEFERRED_ROUTINE)_TimerDpc0;
	timer_dpcs[1] = (PKDEFERRED_ROUTINE)_TimerDpc1;
	timer_dpcs[2] = (PKDEFERRED_ROUTINE)_TimerDpc2;
	timer_dpcs[3] = (PKDEFERRED_ROUTINE)_TimerDpc3;
	timer_dpcs[4] = (PKDEFERRED_ROUTINE)_TimerDpc4;

	my_jiffies=0;
	num_irqs=0;
	my_current=&act_cur;
	pci_probe_dev=probe_dev;

	for(n=0;n<MAX_IRQS;n++)
	{
		reg_irqs[n].handler=NULL;
		reg_irqs[n].irq=-1;
	}
	drvs_num=0;
	need_wakeup=0;
	for(n=0;n<MAX_DRVS;n++)
		m_drivers[n]=NULL;
		
	init_dma(probe_dev->dev_ext);
}
/*------------------------------------------------------------------------*/ 
void handle_irqs(int irq)
{
	int n;
	//printk("handle irqs\n");
	for(n=0;n<MAX_IRQS;n++)
	{
		if (reg_irqs[n].handler && (irq==reg_irqs[n].irq || irq==-1))
			reg_irqs[n].handler(reg_irqs[n].irq,reg_irqs[n].data,NULL);
	}
}
/*------------------------------------------------------------------------*/ 
void inc_jiffies(int n)
{
	my_jiffies+=n;
}
/*------------------------------------------------------------------------*/ 
void do_all_timers(void)
{
	int n;
	for(n=0;n<MAX_TIMERS;n++)
	{
		if (main_timer_list[n] && main_timer_list[n]->function) 
		{
			void (*function)(unsigned long)=main_timer_list[n]->function;
			unsigned long data=main_timer_list[n]->data;
			
			if (main_timer_list[n]->expires>1) {
				main_timer_list[n]->expires--;
			} else {
				
				main_timer_list[n]->expires=0;
				main_timer_list[n]=0; // remove timer
				// Call Timer Function Data
				function(data);
			}
		}
	}
}
/*------------------------------------------------------------------------*/ 
// Purpose: Remember thread procedure and data in global var
// ReactOS Purpose: Create real kernel thread
int my_kernel_thread(int (STDCALL *handler)(void*), void* parm, int flags)
{
	HANDLE hThread = NULL;
	//thread_handler=handler;
	//thread_parm=parm;
	//return 42; // PID :-)
	
	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	PsCreateSystemThread(&hThread,
			     THREAD_ALL_ACCESS,
			     NULL,
			     NULL,
			     NULL,
			     (PKSTART_ROUTINE)handler,
				 parm);

	DPRINT1("usbcore: Created system thread %d\n", (int)hThread);

    return (int)hThread; // FIXME: Correct?
}

// Kill the process
int my_kill_proc(int pid, int signal, int unk)
{
	HANDLE hThread;

	// TODO: Implement actual process killing

	hThread = (HANDLE)pid;
	ZwClose(hThread);

	return 0;
}

/*------------------------------------------------------------------------*/ 
/* Device management
 * As simple as possible, but as complete as necessary ...
 */
/*------------------------------------------------------------------------*/ 


/* calls probe function for hotplug (which does device matching), this is the
   only link between usbcore and the registered device drivers! */
int my_device_add(struct device *dev)
{
	int n,found=0;
	//printk("drv_num %i %p %p\n",drvs_num,m_drivers[0]->probe,m_drivers[1]->probe);
	printk("drv_num %i %p\n",drvs_num,m_drivers[0]->probe);

	if (dev->driver)
	{
		if (dev->driver->probe)
			return dev->driver->probe(dev);
	}
	else
	{
		for(n=0;n<drvs_num;n++)
		{
			if (m_drivers[n]->probe)
			{
				dev->driver=m_drivers[n];
				printk("probe%i %p\n",n,m_drivers[n]->probe);
				if (m_drivers[n]->probe(dev) == 0)
				{
//					return 0;
					found=1;
				}
			}
		}
		if (found) return 0;
	}
	dev->driver=NULL;
	return -ENODEV;
}
/*------------------------------------------------------------------------*/ 
int my_driver_register(struct device_driver *driver)
{

	if (drvs_num<MAX_DRVS)
	{
		printk("driver_register %i: %p %p",drvs_num,driver,driver->probe);  

		m_drivers[drvs_num++]=driver;
		return 0;
	}
	return -1;
}
/*------------------------------------------------------------------------*/ 
int my_device_unregister(struct device *dev)
{
	if (dev->driver && dev->driver->remove)
		dev->driver->remove(dev);
	return 0;
		
}
/*------------------------------------------------------------------------*/ 
struct device *my_get_device(struct device *dev)
{
	return NULL;
}
/*------------------------------------------------------------------------*/ 
void my_device_initialize(struct device *dev)
{
}
/*------------------------------------------------------------------------*/ 
void my_wake_up(PKEVENT evnt)
{
	//DPRINT1("wake_up(), evnt=%p\n", evnt);
	need_wakeup=1;

	KeSetEvent(evnt, 0, FALSE); // Signal event
}
/*------------------------------------------------------------------------*/ 
void my_init_waitqueue_head(PKEVENT evnt)
{
	// this is used only in core/message.c, and it isn't needed there
	//KeInitializeEvent(evnt, NotificationEvent, TRUE); // signalled state
	DPRINT1("init_waitqueue_head(), evnt=%p\n", evnt);
}
/*------------------------------------------------------------------------*/ 
/* wait until woken up (only one wait allowed!) */
extern unsigned int LAST_USB_IRQ;

int my_schedule_timeout(int x)
{
	LONGLONG HH;
	LARGE_INTEGER delay;

	printk("schedule_timeout: %d ms\n", x);

		//delay.QuadPart = -x*10000; // convert to 100ns units
		//KeDelayExecutionThread(KernelMode, FALSE, &delay); //wait_us(1);

	/*
	x+=5; // safety
	x = x*1000;	// to us format
	*/
	x = 50; // it's enough for most purposes

	while(x>0)
	{
		KeQueryTickCount((LARGE_INTEGER *)&HH);//IoInputDword(0x8008);

		do_all_timers();
		handle_irqs(-1);
		
		if (need_wakeup)
			break;

		delay.QuadPart = -10;
		KeDelayExecutionThread(KernelMode, FALSE, &delay); //wait_us(1);
		x-=1;
		//DPRINT("schedule_timeout(): time left: %d\n", x);
	}
	need_wakeup=0;

	printk("schedule DONE!!!!!!\n");

	return 0;//x;
}
/*------------------------------------------------------------------------*/ 
void my_wait_for_completion(struct completion *x)
{
	//printk("wait for completion, x=0x%08x, x->done=%d\n", (DWORD)x, x->done);
	KeWaitForSingleObject(&x->wait, Executive, KernelMode, FALSE, NULL);
	KeClearEvent(&x->wait);
	x->done--;
	//printk("wait for completion done %i\n",x->done);
}
/*------------------------------------------------------------------------*/ 
void my_init_completion(struct completion *x)
{
	//DPRINT1("init_completion(), x=%p\n", x);
	x->done=0;
	KeInitializeEvent(&x->wait, NotificationEvent, FALSE);
}
/*------------------------------------------------------------------------*/ 
void my_interruptible_sleep_on(PKEVENT evnt)
{
	DPRINT1("interruptible_sleep_on(), evnt=%p\n", evnt);
	KeWaitForSingleObject(evnt, Executive, KernelMode, FALSE, NULL);
	KeClearEvent(evnt); // reset to not-signalled
}

// Some kind of a hack currently
void my_try_to_freeze()
{
	LARGE_INTEGER delay;
	delay.QuadPart = -100000; // 0.1 seconds
	KeDelayExecutionThread(KernelMode, FALSE, &delay);
}

/*------------------------------------------------------------------------*/ 
// Helper for pci_module_init
/*------------------------------------------------------------------------*/ 
int my_pci_module_init(struct pci_driver *x)
{
	struct pci_dev *dev=pci_probe_dev;
	const struct pci_device_id *id=NULL;
	if (!pci_probe_dev)
	{
		DPRINT1("PCI device not set!\n");
		return 0;
	}
	x->probe(dev, id);
	return 0;
}
/*------------------------------------------------------------------------*/ 
struct pci_dev *my_pci_find_slot(int a,int b)
{
	return NULL;
}
/*------------------------------------------------------------------------*/ 
int my_request_irq(unsigned int irq,
                       int  (*handler)(int,void *, struct pt_regs *),
                       unsigned long mode, const char *desc, void *data)
{
	if (num_irqs<MAX_IRQS)
	{
		reg_irqs[num_irqs].handler=handler;
		reg_irqs[num_irqs].irq=irq;
		reg_irqs[num_irqs].data=data;
		num_irqs++;
		return 0;
	}
	
	return 1;
}
/*------------------------------------------------------------------------*/ 
int my_free_irq(int irq, void* p)
{
	/* No free... */
	return 0;
}
/*------------------------------------------------------------------------*/ 
// Lookaside funcs
/*------------------------------------------------------------------------*/ 
kmem_cache_t *my_kmem_cache_create(const char *tag, size_t alloc_size,
								   size_t offset, unsigned long flags,
								   void *ctor,
								   void *dtor)
{
	//TODO: Take in account ctor and dtor - callbacks for alloc/free, flags and offset
	//FIXME: We assume this cache is always NPaged
	PNPAGED_LOOKASIDE_LIST Lookaside;
	ULONG Tag=0x11223344; //FIXME: Make this from tag

	Lookaside = ExAllocatePool(NonPagedPool, sizeof(NPAGED_LOOKASIDE_LIST));
	
	ExInitializeNPagedLookasideList(
		Lookaside,
		NULL,
		NULL,
		0,
		alloc_size,
		Tag,
		0);

	return (kmem_cache_t *)Lookaside;
}
/*------------------------------------------------------------------------*/ 
BOOLEAN my_kmem_cache_destroy(kmem_cache_t *co)
{
	ExDeleteNPagedLookasideList((PNPAGED_LOOKASIDE_LIST)co);

	ExFreePool(co);
	return FALSE;
}
/*------------------------------------------------------------------------*/ 
void *my_kmem_cache_alloc(kmem_cache_t *co, int flags)
{
	return ExAllocateFromNPagedLookasideList((PNPAGED_LOOKASIDE_LIST)co);
}
/*------------------------------------------------------------------------*/ 
void my_kmem_cache_free(kmem_cache_t *co, void *ptr)
{
	ExFreeToNPagedLookasideList((PNPAGED_LOOKASIDE_LIST)co, ptr);
}
/*------------------------------------------------------------------------*/ 
// DMA support routines
/*------------------------------------------------------------------------*/ 
#ifdef USB_DMA_SINGLE_SUPPORT
static IO_ALLOCATION_ACTION NTAPI MapRegisterCallback(PDEVICE_OBJECT DeviceObject,
                                                      PIRP Irp,
                                                      PVOID MapRegisterBase,
                                                      PVOID Context);
#endif

NTSTATUS
init_dma(PUSBMP_DEVICE_EXTENSION pDevExt)
{
	// Prepare device descriptor structure
	DEVICE_DESCRIPTION dd;

	RtlZeroMemory( &dd, sizeof(dd) );
	dd.Version = DEVICE_DESCRIPTION_VERSION;
	dd.Master = TRUE;
	dd.ScatterGather = TRUE;
	dd.DemandMode = FALSE;
	dd.AutoInitialize = FALSE;
	dd.Dma32BitAddresses = TRUE;
	dd.InterfaceType = PCIBus;
	dd.DmaChannel = 0;//pDevExt->dmaChannel;
	dd.MaximumLength = 128;//MAX_DMA_LENGTH;
	dd.DmaWidth = Width32Bits;
	dd.DmaSpeed = MaximumDmaSpeed;

	// The following taken from Win2k DDB:
	// "Compute the maximum number of mapping regs
	// this device could possibly need. Since the
	// transfer may not be paged aligned, add one
	// to allow the max xfer size to span a page."
	//pDevExt->mapRegisterCount = (MAX_DMA_LENGTH / PAGE_SIZE) + 1;

    // TODO: Free it somewhere (PutDmaAdapter)
	pDevExt->pDmaAdapter =
		IoGetDmaAdapter( pDevExt->PhysicalDeviceObject,
		&dd,
		&pDevExt->mapRegisterCount);
		
	DPRINT1("IoGetDmaAdapter done 0x%X, mapRegisterCount=%d\n", pDevExt->pDmaAdapter, pDevExt->mapRegisterCount);

	// Fail if failed
	if (pDevExt->pDmaAdapter == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

	return STATUS_SUCCESS;
}

/*
 Timer DPCs
*/
void STDCALL _TimerDpc0(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
	int n = 0;
	void (*function)(unsigned long)=main_timer_list[n]->function;
	unsigned long data=main_timer_list[n]->data;
	//printk("TimerDpc0\n");
	function(data);
	//handle_irqs(-1);
}

void STDCALL _TimerDpc1(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
	int n = 1;
	void (*function)(unsigned long)=main_timer_list[n]->function;
	unsigned long data=main_timer_list[n]->data;
	printk("TimerDpc1\n");
	function(data);
}

void STDCALL _TimerDpc2(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
	int n = 2;
	void (*function)(unsigned long)=main_timer_list[n]->function;
	unsigned long data=main_timer_list[n]->data;
	printk("TimerDpc2\n");
	//function(data);
}

void STDCALL _TimerDpc3(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
	int n = 3;
	void (*function)(unsigned long)=main_timer_list[n]->function;
	unsigned long data=main_timer_list[n]->data;
	printk("TimerDpc3\n");
	//function(data);
}

void STDCALL _TimerDpc4(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
	int n = 4;
	void (*function)(unsigned long)=main_timer_list[n]->function;
	unsigned long data=main_timer_list[n]->data;
	printk("TimerDpc4\n");
	//function(data);
}

void *my_dma_pool_alloc(struct dma_pool *pool, int gfp_flags, dma_addr_t *dma_handle)
{
	// HalAllocCommonBuffer
	// But ideally IoGetDmaAdapter

	DPRINT1("dma_pool_alloc() called\n");
	return NULL;
}

/*
pci_pool_create --  Creates a pool of pci consistent memory blocks, for dma. 

struct pci_pool * pci_pool_create (const char * name, struct pci_dev * pdev, size_t size, size_t align, size_t allocation, int flags);

Arguments:
name - name of pool, for diagnostics 
pdev - pci device that will be doing the DMA 
size - size of the blocks in this pool. 
align - alignment requirement for blocks; must be a power of two 
allocation - returned blocks won't cross this boundary (or zero) 
flags - SLAB_* flags (not all are supported). 

Description:
Returns a pci allocation pool with the requested characteristics, or null if one can't be created.
Given one of these pools, pci_pool_alloc may be used to allocate memory. Such memory will all have
"consistent" DMA mappings, accessible by the device and its driver without using cache flushing
primitives. The actual size of blocks allocated may be larger than requested because of alignment. 
If allocation is nonzero, objects returned from pci_pool_alloc won't cross that size boundary.
This is useful for devices which have addressing restrictions on individual DMA transfers, such
as not crossing boundaries of 4KBytes. 
*/
struct pci_pool *my_pci_pool_create(const char * name, struct device * pdev, size_t size, size_t align, size_t allocation)
{
	struct pci_pool		*retval;

	if (align == 0)
		align = 1;
	if (size == 0)
		return 0;
	else if (size < align)
		size = align;
	else if ((size % align) != 0) {
		size += align + 1;
		size &= ~(align - 1);
	}

	if (allocation == 0) {
		if (PAGE_SIZE < size)
			allocation = size;
		else
			allocation = PAGE_SIZE;
		// FIXME: round up for less fragmentation
	} else if (allocation < size)
		return 0;
		
	retval = ExAllocatePool(NonPagedPool, sizeof(struct pci_pool)); // Non-paged because could be
																	// accesses at IRQL < PASSIVE

	// fill retval structure
	strncpy (retval->name, name, sizeof retval->name);
	retval->name[sizeof retval->name - 1] = 0;
	
	retval->allocation = allocation;
	retval->size = size;
	retval->blocks_per_page = allocation / size;
	retval->pdev = pdev;

	retval->pages_allocated = 0;
	retval->blocks_allocated = 0;
	
	DPRINT("pci_pool_create(): %s/%s size %d, %d/page (%d alloc)\n",
		pdev ? pdev->name : NULL, retval->name, size,
		retval->blocks_per_page, allocation);

	return retval;
}

/*
Name:
pci_pool_alloc --  get a block of consistent memory 

Synopsis:
void * pci_pool_alloc (struct pci_pool * pool, int mem_flags, dma_addr_t * handle);

Arguments:
pool - pci pool that will produce the block 

mem_flags - SLAB_KERNEL or SLAB_ATOMIC 

handle - pointer to dma address of block 

Description:
This returns the kernel virtual address of a currently unused block, and reports its dma
address through the handle. If such a memory block can't be allocated, null is returned. 
*/
void * my_pci_pool_alloc(struct pci_pool * pool, int mem_flags, dma_addr_t *dma_handle)
{
	PVOID result;
	PUSBMP_DEVICE_EXTENSION devExt = (PUSBMP_DEVICE_EXTENSION)to_pci_dev(pool->pdev)->dev_ext;
	int page=0, offset;
	int map, i, block;

	//DPRINT1("pci_pool_alloc() called, blocks already allocated=%d, dma_handle=%p\n", pool->blocks_allocated, dma_handle);
	//ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	if (pool->pages_allocated == 0)
	{
		// we need to allocate at least one page
		pool->pages[pool->pages_allocated].virtualAddress =
			devExt->pDmaAdapter->DmaOperations->AllocateCommonBuffer(devExt->pDmaAdapter,
				PAGE_SIZE, &pool->pages[pool->pages_allocated].dmaAddress, FALSE); //FIXME: Cache-enabled?

		// mark all blocks as free (bit=set)
		memset(pool->pages[pool->pages_allocated].bitmap, 0xFF, 128*sizeof(unsigned long));

		/* FIXME: the next line replaces physical address by virtual address:
		* this change is needed to boot VMWare, but I'm really not sure this
		* change is correct!
		*/
		//pool->pages[pool->pages_allocated].dmaAddress.QuadPart = (ULONG_PTR)pool->pages[pool->pages_allocated].virtualAddress;
		pool->pages_allocated++;
	}

	// search for a free block in all pages
	for (page=0; page<pool->pages_allocated; page++)
	{
		for (map=0,i=0; i < pool->blocks_per_page; i+= BITS_PER_LONG, map++)
		{
			if (pool->pages[page].bitmap[map] == 0)
				continue;
            
			block = ffz(~ pool->pages[page].bitmap[map]);

			if ((i + block) < pool->blocks_per_page)
			{
				//DPRINT("pci_pool_alloc(): Allocating block %p:%d:%d:%d\n", pool, page, map, block);
				clear_bit(block, &pool->pages[page].bitmap[map]);
				offset = (BITS_PER_LONG * map) + block;
				offset *= pool->size;
				goto ready;
			}
		}
	}

	//TODO: alloc page here then
	DPRINT1("Panic!! We need one more page to be allocated, and Fireball doesn't want to alloc it!\n");
	offset = 0;
	return 0;

ready:
	*dma_handle = pool->pages[page].dmaAddress.QuadPart + offset;
	result = (char *)pool->pages[page].virtualAddress + offset;
	pool->blocks_allocated++;

	return result;
}

/*
Name
pci_pool_free --  put block back into pci pool 
Synopsis

void pci_pool_free (struct pci_pool * pool, void * vaddr, dma_addr_t dma);

Arguments

pool - the pci pool holding the block 

vaddr - virtual address of block 

dma - dma address of block 

Description:
Caller promises neither device nor driver will again touch this block unless it is first re-allocated.
*/
void my_pci_pool_free (struct pci_pool * pool, void * vaddr, dma_addr_t dma)
{
	int page, block, map;

	// Find page
	for (page=0; page<pool->pages_allocated; page++)
	{
		if (dma < pool->pages[page].dmaAddress.QuadPart)
			continue;
		if (dma < (pool->pages[page].dmaAddress.QuadPart + pool->allocation))
			break;
	}

	block = dma - pool->pages[page].dmaAddress.QuadPart;
	block /= pool->size;
	map = block / BITS_PER_LONG;
	block %= BITS_PER_LONG;

	// mark as free
	set_bit (block, &pool->pages[page].bitmap[map]);

	pool->blocks_allocated--;
	DPRINT("pci_pool_free(): alloc'd: %d\n", pool->blocks_allocated);
}

/*
pci_pool_destroy --  destroys a pool of pci memory blocks. 
Synopsis

void pci_pool_destroy (struct pci_pool * pool);


Arguments:
pool - pci pool that will be destroyed 

Description
Caller guarantees that no more memory from the pool is in use, and that nothing will try to
use the pool after this call. 
*/
void my_pci_pool_destroy (struct pci_pool * pool)
{
	DPRINT1("pci_pool_destroy(): alloc'd: %d, UNIMPLEMENTED\n", pool->blocks_allocated);

	ExFreePool(pool);
}

// the code here is identical to dma_alloc_coherent
void  *my_pci_alloc_consistent(struct pci_dev *hwdev, size_t size, dma_addr_t *dma_handle)
{
    PUSBMP_DEVICE_EXTENSION devExt = (PUSBMP_DEVICE_EXTENSION)hwdev->dev_ext;
	DPRINT1("pci_alloc_consistent() size=%d, dma_handle=%p\n", size, (PPHYSICAL_ADDRESS)dma_handle);

    return devExt->pDmaAdapter->DmaOperations->AllocateCommonBuffer(devExt->pDmaAdapter, size, (PPHYSICAL_ADDRESS)dma_handle, FALSE); //FIXME: Cache-enabled?
}

dma_addr_t my_dma_map_single(struct device *hwdev, void *ptr, size_t size, enum dma_data_direction direction)
{
    //PHYSICAL_ADDRESS BaseAddress;
    //PUSBMP_DEVICE_EXTENSION pDevExt = (PUSBMP_DEVICE_EXTENSION)hwdev->dev_ext;
    //PUCHAR VirtualAddress = (PUCHAR) MmGetMdlVirtualAddress(pDevExt->Mdl);
	//ULONG transferSize = size;
	//BOOLEAN WriteToDevice;

	//DPRINT1("dma_map_single() ptr=0x%lx, size=0x%x, dir=%d\n", ptr, size, direction);
	/*ASSERT(pDevExt->BufferSize > size);

	// FIXME: It must be an error if DMA_BIDIRECTIONAL trasnfer happens, since MSDN says
	//        the buffer is locked
	if (direction == DMA_BIDIRECTIONAL || direction == DMA_TO_DEVICE)
        WriteToDevice = TRUE;
	else
		WriteToDevice = FALSE;

    DPRINT1("IoMapTransfer\n");
    BaseAddress = pDevExt->pDmaAdapter->DmaOperations->MapTransfer(pDevExt->pDmaAdapter,
                    pDevExt->Mdl,
                    pDevExt->MapRegisterBase,
                    (PUCHAR) MmGetMdlVirtualAddress(pDevExt->Mdl),
                    &transferSize,
                    WriteToDevice);

	if (WriteToDevice)
	{
		DPRINT1("Writing to the device...\n");
		memcpy(VirtualAddress, ptr, size);
	}
	else
	{
		DPRINT1("Reading from the device...\n");
		memcpy(ptr, VirtualAddress, size);
	}*/

	//DPRINT1("VBuffer == 0x%x (really 0x%x?) transf_size == %u\n", pDevExt->VirtualBuffer, MmGetPhysicalAddress(pDevExt->VirtualBuffer).LowPart, transferSize);
	//DPRINT1("VBuffer == 0x%x (really 0x%x?) transf_size == %u\n", ptr, MmGetPhysicalAddress(ptr).LowPart, transferSize);
	
	return MmGetPhysicalAddress(ptr).QuadPart;//BaseAddress.QuadPart; /* BIG HACK */
}

// 2.6 version of pci_unmap_single
//void my_dma_unmap_single(struct device *dev, dma_addr_t dma_addr, size_t size, enum dma_data_direction direction)
void my_dma_unmap_single(struct device *dev, dma_addr_t dma_addr, size_t size, enum dma_data_direction direction)
{
	//DPRINT1("dma_unmap_single() called, nothing to do\n");
	/* nothing yet */
}

void my_dma_sync_single(struct device *hwdev,
				       dma_addr_t dma_handle,
				       size_t size, int direction)
{
	DPRINT1("dma_sync_single() called, UNIMPLEMENTED\n");
	/* nothing yet */
}

void my_dma_sync_sg(struct device *hwdev,
				   struct scatterlist *sg,
				   int nelems, int direction)
{
	DPRINT1("dma_sync_sg() called, UNIMPLEMENTED\n");
	/* nothing yet */
}


int my_dma_map_sg(struct device *hwdev, struct scatterlist *sg, int nents, enum dma_data_direction direction)
{
	DPRINT1("dma_map_sg() called, UNIMPLEMENTED\n");
	return 0;
}

void my_dma_unmap_sg(struct device *hwdev, struct scatterlist *sg, int nents, enum dma_data_direction direction)
{
	DPRINT1("dma_unmap_sg() called, UNIMPLEMENTED\n");
	/* nothing yet */
}

/* forwarder ro dma_ equivalent */
void my_pci_unmap_single(struct pci_dev *hwdev, dma_addr_t dma_addr, size_t size, int direction)
{
	my_dma_unmap_single(&hwdev->dev, dma_addr, size, direction);
}

// the code here is very similar to pci_alloc_consistent()
void *my_dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flag)
{
	//struct pci_dev *pdev = to_pci_dev(dev);
	//return my_pci_alloc_consistent(pdev, sz, dma_handle);

    PUSBMP_DEVICE_EXTENSION devExt = (PUSBMP_DEVICE_EXTENSION)dev->dev_ext;
	DPRINT1("dma_alloc_coherent() size=%d, dev=%p, dev_ext=%p, dma_handle=%p, DmaAdapter=%p\n", size, dev, dev->dev_ext, (PPHYSICAL_ADDRESS)dma_handle, devExt->pDmaAdapter);

    return devExt->pDmaAdapter->DmaOperations->AllocateCommonBuffer(devExt->pDmaAdapter, size, (PPHYSICAL_ADDRESS)dma_handle, FALSE); //FIXME: Cache-enabled?

}
/*
void dma_free_coherent(struct device *dev, size_t size,
			 void *vaddr, dma_addr_t dma_handle); */



/*------------------------------------------------------------------------*/ 
/* SPINLOCK routines                                                      */
/*------------------------------------------------------------------------*/ 
void my_spin_lock_init(spinlock_t *sl)
{
	//DPRINT("spin_lock_init: %p\n", sl);
	KeInitializeSpinLock(&sl->SpinLock);
}

void my_spin_lock(spinlock_t *sl)
{
	//DPRINT("spin_lock_: %p\n", sl);
	KeAcquireSpinLock(&sl->SpinLock, &sl->OldIrql);
}

void my_spin_unlock(spinlock_t *sl)
{
	//DPRINT("spin_unlock: %p\n", sl);
	KeReleaseSpinLock(&sl->SpinLock, sl->OldIrql);
}

void my_spin_lock_irqsave(spinlock_t *sl, int flags)
{
	//DPRINT("spin_lock_irqsave: %p\n", sl);
	//my_spin_lock(sl);
	KeAcquireSpinLock(&sl->SpinLock, &sl->OldIrql);
}

void my_spin_unlock_irqrestore(spinlock_t *sl, int flags)
{
	//DPRINT("spin_unlock_irqrestore: %p\n", sl);
	//my_spin_unlock(sl);
	KeReleaseSpinLock(&sl->SpinLock, sl->OldIrql);
}

void my_spin_lock_irq(spinlock_t *sl)
{
	//DPRINT("spin_lock_irq: %p\n", sl);
	//my_spin_lock(sl);
	KeAcquireSpinLock(&sl->SpinLock, &sl->OldIrql);
}

void my_spin_unlock_irq(spinlock_t *sl)
{
	//DPRINT("spin_unlock_irq: %p\n", sl);
	//my_spin_unlock(sl);
	KeReleaseSpinLock(&sl->SpinLock, sl->OldIrql);
}



/*------------------------------------------------------------------------*/ 
/* Misc routines                                                          */
/*------------------------------------------------------------------------*/ 
/**
 * strlcpy - Copy a %NUL terminated string into a sized buffer
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @size: size of destination buffer
 *
 * Compatible with *BSD: the result is always a valid
 * NUL-terminated string that fits in the buffer (unless,
 * of course, the buffer size is zero). It does not pad
 * out the result like strncpy() does.
 */
size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size-1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}

/**
 * kzalloc - allocate memory. The memory is set to zero.
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate.
 */
void *my_kzalloc(size_t size/*, gfp_t flags*/)
{
	void *ret = kmalloc(size, flags);
	if (ret)
		memset(ret, 0, size);
	return ret;
}

/**
 * memcmp - Compare two areas of memory
 * @cs: One area of memory
 * @ct: Another area of memory
 * @count: The size of the area.
 */
int __cdecl memcmp(const void * cs,const void * ct,size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

/*------------------------------------------------------------------------*/ 
int my_pci_write_config_word(struct pci_dev *dev, int where, u16 val)
{
	//dev->bus, dev->devfn, where, val
	PUSBMP_DEVICE_EXTENSION dev_ext = (PUSBMP_DEVICE_EXTENSION)dev->dev_ext;

	//FIXME: Is returning this value correct?
	//FIXME: Mixing pci_dev and win structs isn't a good thing at all (though I doubt it wants to access device in another slot/bus)
	DPRINT1("pci_write_config_word: BusNum: %d, SlotNum: 0x%x, value: 0x%x, where: 0x%x\n",
		dev_ext->SystemIoBusNumber, dev_ext->SystemIoSlotNumber.u.AsULONG, val, where);
	return HalSetBusDataByOffset(PCIConfiguration, dev_ext->SystemIoBusNumber, dev_ext->SystemIoSlotNumber.u.AsULONG, &val, where, sizeof(val));
}

/*------------------------------------------------------------------------*/ 
int my_pci_read_config_word(struct pci_dev *dev, int where, u16 *val)
{
	ULONG result;

	//dev->bus, dev->devfn, where, val
	PUSBMP_DEVICE_EXTENSION dev_ext = (PUSBMP_DEVICE_EXTENSION)dev->dev_ext;

	//FIXME: Is returning this value correct?
	//FIXME: Mixing pci_dev and win structs isn't a good thing at all
	result = HalGetBusDataByOffset(PCIConfiguration, dev_ext->SystemIoBusNumber, dev_ext->SystemIoSlotNumber.u.AsULONG, val, where, sizeof(*val));
	DPRINT1("pci_read_config_word: BusNum: %d, SlotNum: 0x%x, value: 0x%x, where: 0x%x\n",
		dev_ext->SystemIoBusNumber, dev_ext->SystemIoSlotNumber.u.AsULONG, *val, where);

	return result;
}

/* For compatibility with Windows XP */
NTSTATUS
_RtlDuplicateUnicodeString(
   IN ULONG Flags,
   IN PCUNICODE_STRING SourceString,
   OUT PUNICODE_STRING DestinationString)
{
   if (SourceString == NULL || DestinationString == NULL)
      return STATUS_INVALID_PARAMETER;


   if ((SourceString->Length == 0) && 
       (Flags != (RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE | 
                  RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING)))
   {
      DestinationString->Length = 0;
      DestinationString->MaximumLength = 0;
      DestinationString->Buffer = NULL;
   }
   else
   {
      ULONG DestMaxLength = SourceString->Length;

      if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)
         DestMaxLength += sizeof(UNICODE_NULL);

      DestinationString->Buffer = ExAllocatePool(PagedPool, DestMaxLength);
      if (DestinationString->Buffer == NULL)
         return STATUS_NO_MEMORY;

      RtlCopyMemory(DestinationString->Buffer, SourceString->Buffer, SourceString->Length);
      DestinationString->Length = SourceString->Length;
      DestinationString->MaximumLength = DestMaxLength;

      if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)
         DestinationString->Buffer[DestinationString->Length / sizeof(WCHAR)] = 0;
   }

   return STATUS_SUCCESS;
}
