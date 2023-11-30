#include <stdio.h>
#include <stdlib.h>


#define BUFFER_SIZE 1024
#define FILE_SIZE 1000
#define PAGES 256
#define PAGE_SIZE 256
#define FRAMES 256
#define FRAME_SIZE 256
#define TLB_SIZE 16

/*
The struct makes it easier to hold multiple data for each address.
*/
struct address_components {
    int logical_address;
    int page_number;
    int frame_number; // Actually redundant but useful during development
    int offset;
    int physical_address;
};
struct address_components addresses[FILE_SIZE];

int page_table[PAGES];

/* 
Simulated main memory (RAM). pm_head is a pointer for the main memory. 
*/
int physical_memory[FRAMES][FRAME_SIZE];
int pm_head;

/*
Holds up to 16 {page number, frame number} touples. tlb_head is a pointer for the TLB.
*/
int tlb[TLB_SIZE][2];
int tlb_amount;

/*
The percentage of address references that resulted in page faults and the 
percentage of address references that were resolved in the TLB.
*/
int page_fault_count;
int tlb_hit_count;

/* 
This function extracts the offset from a logical address.
*/
int extractOffset(int logical_address) {
    int offset = logical_address & 0xFF;
    return offset;
}

/*
This function extracts the page number from a logical address.
*/
int extractPageNumber(int logical_address) {
    int page_number = (logical_address >> 8) & 0xFF;
    return page_number;
}

/*
Reads required bytes from the BACKING_STORE.bin file and returns the appropriate frame. This
emulates the disk storage.
*/
int readBin(int page_number) {
    FILE *bin_handle = fopen("./data/BACKING_STORE.bin", "rb");
    if(bin_handle == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    unsigned char buffer[FRAME_SIZE];
    fseek(bin_handle, (page_number * 256), SEEK_SET); // Set the pointer to the location of the page
    // Now, read the bytes from the disk and copy it to main memory
    fread(buffer, 1, 256, bin_handle);
    for(int i = 0; i < 256; i++){
        physical_memory[pm_head][i] = buffer[i];
    }
    page_table[page_number] = pm_head;
    pm_head++;
    fclose(bin_handle);
    return page_table[page_number];
}

/* 
Updates the TLB with FIFO replacement strategy.
*/
void updateTLB(int page_number, int frame) {
    if (tlb_amount == TLB_SIZE-1) {
        for(int i = 0; i < TLB_SIZE-1; i++) {
            tlb[i][0] = tlb[i+1][0];
            tlb[i][1] = tlb[i+1][1];
        }
        tlb[TLB_SIZE-1][0] = page_number;
        tlb[TLB_SIZE-1][1] = frame;
        tlb_amount = 0;
    }
    else {
        tlb[tlb_amount][0] = page_number;
        tlb[tlb_amount][1] = frame;
        tlb_amount++;
    }
}

/*
Searches the TLB (Cache) if the page exists and then returns the frame number.
*/
int searchTLB(int page_number) {
    for(int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i][0] == page_number) {
            return tlb[i][1];
        } 
    }
    return -1;
}


/*
Attempts to find the physical address with the TLB. If the address is not found then we consult
the page table. In the case of page fault then we would read from the disk storage. 
*/
int findPhysicalAddress(struct address_components address) {
    int frame = searchTLB(address.page_number);
    // Check if in TLB WIP
    if(frame != -1) {
        tlb_hit_count++;
        address.frame_number = frame;
    }
    else {
        // Check the page table
        if (page_table[address.page_number] != -1) {
            frame = page_table[address.page_number];
            address.frame_number = frame;
        } 
        else {
            page_fault_count++;
            // Load in frame from bin
            frame = readBin(address.page_number);
            page_table[address.page_number] = frame;
            address.frame_number = frame;
        }
        updateTLB(address.page_number, frame);
    }
    
    int physical_address = (frame << 8) + address.offset;
    //printf("Virtual address: %d Physical address: %d Page Number: %d Frame: %d Offset: %d\n", address.logical_address, physical_address, address.page_number, frame, address.offset);

    return physical_address;
}

/*
Controlls the main program flow.
*/
int main(int argc, char *argv[]) {
    // Open the file specified in argv
    FILE *file_handle = fopen(argv[1], "r");
    if(file_handle == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    } 
    pm_head = 0;
    page_fault_count = 0;
    tlb_hit_count = 0;
    tlb_amount = 0;

    // Initialize the page table and tlb with "false" values 
    for (int i = 0; i < 256; i++) {
        page_table[i] = -1;
    }
    for (int j = 0; j < 16; j++) {
        tlb[j][0] = -1;
        tlb[j][1] = -1;
    }
        
    // Parse the text file and call the extractors to fill our struct array
    char buffer[BUFFER_SIZE];
    int index = 0;
    int number; 
    while(fgets(buffer, BUFFER_SIZE - 1, file_handle) != NULL) {
        number = atoi(buffer);
        addresses[index].logical_address = number;
        addresses[index].page_number = extractPageNumber(number);
        addresses[index].offset = extractOffset(number);
        index++;
    }
    fclose(file_handle);

    // Debug
    /*
    for(int t = 0; t < FILE_SIZE; t++) {
       printf("Virtual address: %d Page number: %d Offset: %d\n", addresses[t].logical_address, addresses[t].page_number, addresses[t].offset);
    }
    */

    // Start the physical address conversion process
    for(int k = 0; k < FILE_SIZE; k++) {
        addresses[k].physical_address = findPhysicalAddress(addresses[k]);
    }

    // Final print
    for(int l = 0; l < FILE_SIZE; l++) {
        // Using signed char we don't have to worry about flipping bits and sign extending
        signed char value = physical_memory[page_table[addresses[l].page_number]][addresses[l].offset];
        printf("Virtual address: %d Physical address: %d Value: %d\n", addresses[l].logical_address, addresses[l].physical_address, value);
    }

    // Final statistics as the assignment requires.
    printf("Page-fault rate: %f\n", page_fault_count / 1000.0);
    printf("TLB hit rate: %f\n", tlb_hit_count / 1000.0);
	return 0;
}
