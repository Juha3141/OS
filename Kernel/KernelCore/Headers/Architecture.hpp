#ifndef _ARCHITECTURE_HPP_
#define _ARCHITECTURE_HPP_

// Rachmaninoff Prelude in D major, Op.23 No.4 - Mikhail Pletnev

#define MMODEL_PHYSICAL_MAPPING 0x01 // 1:1 scale of physical memory and virtual memory
#define MMODEL_DEMAND_PAGING    0x02 // Demand Paging

// Only "NOT" for architecture-specific things...

typedef unsigned long max_t;
typedef signed long max_s_t;

// could not work
typedef unsigned long qword;
typedef signed long qword_s;

typedef unsigned int dword;
typedef signed int dword_s;
typedef unsigned short word;
typedef signed short word_s;
typedef unsigned char byte;
typedef signed char byte_s;

typedef unsigned char io_data; // define the data type manually
typedef unsigned short io_port;

typedef unsigned long interrupt_handler;

typedef struct {
    dword id;
}core;

typedef struct {
    dword total_core_cnt;
    dword physical_core_cnt;
    char vendor_id[12];

    char brand_string[48];
    qword CPU_speed;
}general_cpu_info;

typedef struct {
    
}memory_map;

namespace Architecture {
    // I/O port
    io_data IO_read(io_port port);
    io_data IO_write(io_port port);

    // interrupt
    void initialize_interrupt();
    void register_interrupt(dword int_number , interrupt_handler handler);
    void deregister_interrupt(dword int_number);

    // timer system
    void initialize_timer_system(interrupt_handler Handler);

    // segmentation
    bool supportation_segment();

    void initialize_segment();
    void register_segment();
    void deregister_segment();

    // memory model & memory model
    byte system_memory_model(void); // work just as like a constant
    void get_memory_map(memory_map &mmap);

    // paging
    bool supportation_paging();
    
    void initialize_paging();
    void set_page_entry(); // level, page number, properties ...

    // CPU instructions
    void pause(void);
    void halt(void);
    qword rdtsc(void);
    
    // Multiprocessor Specified
    bool supportation_mp();
    
    // CPU information
    bool get_general_cpu_info(general_cpu_info &cpuinfo);
};

#endif