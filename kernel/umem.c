#include "kernel.h"

static void init_second_level(uint32_t *second)
{
	uint32_t i;
	for (i = 0; i < 256; i++)
		second[i] = 0;
}

void umem_map_page(struct process *p, uint32_t virt, uint32_t phys, uint32_t attrs)
{
	uint32_t *second;
	uint32_t first_idx = virt >> 20;
	uint32_t second_idx = (virt >> 12) & 0xFF;

	if ((p->first[first_idx] & FLD_MASK) != FLD_COARSE) {
		second = kmem_get_pages(0x1000, 0);
		p->shadow[first_idx] = second;
		p->first[first_idx] = (kmem_lookup_phys(second) | FLD_COARSE);
		init_second_level(second);
	}
	second = p->shadow[first_idx];

	second[second_idx] = phys & 0xFFFFF000 | attrs | SLD_SMALL | SLD_NG;
}

void umem_map_pages(struct process *p, uint32_t virt, uint32_t phys, uint32_t len, uint32_t attrs)
{
	uint32_t i;
	for (i = 0; i < len; i += 4096)
		umem_map_page(p, virt + i, phys + i, attrs);
}
