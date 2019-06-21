/*
 * mem_internal.c: internal handling for kernel and user memory tables
 */

struct mem {
	uint32_t *base;
	uint32_t *shadow;

	#define STRAT_DIRECT 1
	#define STRAT_SHADOW 2
	uint32_t strategy;
};


/**
 * Print out second level table entries.
 * second: pointer to the second level table
 * start: first virtual address to print entries for
 * end: last virtual address to print entries for (may be outside of range)
 *
 * For start and end, we really only care about the middle bits that determine
 * which second level descriptor to use.
 */
static void print_second_level(struct mem *mem, uint32_t *second, uint32_t start, uint32_t end)
{
	uint32_t virt_base = start & top_n_bits(12);
	uint32_t i = (start >> 12) & 0xFF;

	while (start < end && i < 256) {
		switch (second[i] & SLD_MASK) {
			case SLD_LARGE:
				printf("\t0x%x: 0x%x (large)\n",
					virt_base + (i << 12),
					second[i] & top_n_bits(16)
				);
				break;
			case 0x3:
			case SLD_SMALL:
				printf("\t0x%x: 0x%x (small), xn=%u, tex=%u, ap=%u, apx=%u, ng=%u\n",
					virt_base + (i << 12),
					second[i] & top_n_bits(20),
					second[i] & 0x1,
					(second[i] >> 6) & 0x7,
					(second[i] >> 4) & 0x3,
					(second[i] & (1 << 9)) ? 1 : 0,
					(second[i] & SLD_NG) ? 1 : 0
				);
				break;
			default:
				printf("\t0x%x: unmapped\n", virt_base + (i<<12));
		}
		i += 1;
		start = virt_base | (i << 12);
	}
}

static uint32_t *get_second(struct mem *mem, uint32_t first_idx)
{
	if (mem->strategy == STRAT_DIRECT) {
		/* kernel direct strategy */
		return second_level_table + (i * 1024);
	}
}

static void print_first_level(struct mem *mem, uint32_t start, uint32_t stop)
{
	uint32_t i = start >> 20;
	uint32_t stop_idx = stop >> 20;
	while (i <= stop_idx) {
		switch (mem->base[i] & FLD_MASK) {
			case FLD_SECTION:
				printf("0x%x: 0x%x (section), domain=%u\n",
					i << 20,
					mem->base[i] & top_n_bits(10),
					(mem->base[i] >> 5) & 0xF
				);
				break;
			case FLD_COARSE:
				printf("0x%x: 0x%x (second level), domain=%u\n",
					i << 20,
					mem->base[i] & top_n_bits(22),
					(mem->base[i] >> 5) & 0xF
				);
				print_second_level(
					second_level_table + (i * 1024),
					start, stop
				);
				break;
			case FLD_UNMAPPED:
			default:
				break;
		}
		i += 1;
		start = i << 20;
	}
}
