#include "fio.h"
#include "seg_map.h"

void
insert_seg_map(struct thread_data *td, struct io_piece *ipo)
{
	unsigned int min_bs = td->o.rw_min_bs;
	struct fio_file *f = ipo->file;
	unsigned long long block;
	unsigned int blocks, nr_blocks;
	int busy_check;

	block = (ipo->offset - f->file_offset) / (unsigned long long) min_bs;
	nr_blocks = (ipo->buflen + min_bs - 1) / min_bs;
	blocks = 0;

    unsigned int idx, bit;
    idx = MAP2_IDX(block);
    bit = MAP2_BIT(block);
}
