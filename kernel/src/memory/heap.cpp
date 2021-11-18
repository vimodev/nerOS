#include "heap.h"

#include "../paging/PageTableManager.h"
#include "../paging/PageFrameAllocator.h"

void *heap_start;
void *heap_end;
HeapSegmentHeader *last_header;

// Initialize the heap
void initialize_heap(void *heap_address, size_t page_count) {
    // Map all the required memory for the heap
    void *pos = heap_address;
    for (size_t i = 0; i < page_count; i++) {
        // Grab a physical memory page and map it to the position
        GlobalPageTableManager.map_memory(pos, GlobalAllocator.request_page());
        pos = (void *) ((size_t) pos + 0x1000);
    }
    size_t heap_length = page_count * 0x1000;
    heap_start = heap_address;
    heap_end = (void *) ((size_t) heap_start + heap_length);
    // Heap now consists of one segment
    HeapSegmentHeader *start_segment = (HeapSegmentHeader *) heap_address;
    // Subtract header's own memory requirement to prevent overflow later
    start_segment->length = heap_length - sizeof(HeapSegmentHeader);
    start_segment->next = NULL;
    start_segment->last = NULL;
    start_segment->free = true;
    last_header = start_segment;
}

// Expand the heap by the required amount
void expand_heap(size_t length) {
    // Round the length up to an entire page
    if (length % 0x1000) {
        length -= length % 0x1000;
        length += 0x1000;
    }
    size_t page_count = length / 0x1000;
    // Put new segment at the end of the current heap
    HeapSegmentHeader *new_segment = (HeapSegmentHeader *) heap_end;
    // And allocate all the newly required memory
    for (size_t i = 0; i < page_count; i++) {
        GlobalPageTableManager.map_memory(heap_end, GlobalAllocator.request_page());
        heap_end = (void *) ((size_t) heap_end + 0x1000);
    }
    // Update the heap structure
    new_segment->free = true;                                   // New segment is free
    new_segment->last = last_header;                            // The new segment's previous segment is the total last one
    last_header->next = new_segment;                            // The last segment's next segment is the new segment
    last_header = new_segment;                                  // And the new last segment is the new segment
    new_segment->next = NULL;
    new_segment->length = length - sizeof(HeapSegmentHeader);
    // And we combine the previous segment with the new segment
    new_segment->combine_backward();
}

// Allocate some memory on the heap, and return a pointer to it
void *malloc(size_t size) {
    if (size == 0) return NULL;
    // Must be a multiple of 16 bytes (128 bits)
    if (size % 0x10 > 0) {
        // Round up if necessary
        size -= (size % 0x10);
        size += 0x10;
    }
    // Find a free segment with enough space
    HeapSegmentHeader *current_segment = (HeapSegmentHeader *) heap_start;
    while (true) {
        // Segment must be free
        if (current_segment->free) {
            // And segment must have enough space
            if (current_segment->length > size) {
                // Split the segment on the required size
                current_segment->split(size);
                current_segment->free = false;
                // And return it, but leave space for header
                return (void *)((uint64_t) current_segment + sizeof(HeapSegmentHeader));
            }
            // If it exactly fits
            if (current_segment->length == size) {
                // Perfect fit, allocate the segment to the requestor
                current_segment->free = false;
                return (void *)((uint64_t) current_segment + sizeof(HeapSegmentHeader));
            }
        }
        if (current_segment->next == NULL) break;
        current_segment = current_segment->next;
    }
    // If we couldnt find a proper segment, we must expand the heap
    expand_heap(size);
    // And try again
    return malloc(size);
}

// Free the segment containing the given address
void free(void *address) {
    // Address is the first address after the header
    HeapSegmentHeader *segment = (HeapSegmentHeader *) address - 1;
    // Set it to free
    segment->free = true;
    // And merge forwards and backwards
    segment->combine_forward();
    segment->combine_backward();
}

// Split the heap segment into 2
HeapSegmentHeader *HeapSegmentHeader::split(size_t split_length) {
    if (split_length < 0x10) return NULL;
    int64_t split_segment_length = length - split_length - sizeof(HeapSegmentHeader);
    if (split_segment_length < 0x10) return NULL;
    // New segment starts at an offset of the split length + the header size ||| from the current segment
    HeapSegmentHeader *new_split_header = (HeapSegmentHeader *) ((size_t)this + split_length + sizeof(HeapSegmentHeader));
    // We insert the newly split segment in between this and the next
    next->last = new_split_header;                      // So next segment's last segment will be the new split
    new_split_header->next = next;                      // And our new segment's next segment will be the current next
    next = new_split_header;                            // And the next of our current segment will be the new segment
    new_split_header->last = this;                      // And our new segment's last segment will be the current segment
    new_split_header->length = split_segment_length;    // And our new segment's length will be the calculated split segment length
    new_split_header->free = free;                      // Synchronize the status
    length = split_length;                              // And our current segment will have the desired size
    // And update the total last header if the current segment was the last
    if (last_header == this) last_header = new_split_header;
    return new_split_header;
}

// Combine this segment with the next one
void HeapSegmentHeader::combine_forward() {
    // Must have a next segment and next segment must be free
    if (next == NULL) return;
    if (!next->free) return;
    // If the next segment was the total last one, now this one is
    if (next == last_header) last_header = this;
    // If the next segment has a next segment, update that segment's 'last pointer'
    if (next->next != NULL) {
        next->next->last = this;
    }
    next = next->next;
    // Update the length of this segment to combine both lengths
    // And we now only use 1 header whereas we previously used 2
    length = length + next->length + sizeof(HeapSegmentHeader);
}

// Combine this segment with the previous one
void HeapSegmentHeader::combine_backward() {
    // Just call the previous segment's combine forward function
    if (last != NULL && last->free) last->combine_forward();
}