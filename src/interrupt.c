#include "lib-header/fat32.h"
#include "lib-header/framebuffer.h"
#include "lib-header/idt.h"
#include "lib-header/interrupt.h"
#include "lib-header/keyboard.h"
#include "lib-header/portio.h"
#include "lib-header/stdmem.h"

struct TSSEntry _interrupt_tss_entry = {
    .ss0 = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void io_wait(void)
{
    out(0x80, 0);
}

void pic_ack(uint8_t irq)
{
    if (irq >= 8)
        out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void)
{
    uint8_t a1, a2;

    // Save masks
    a1 = in(PIC1_DATA);
    a2 = in(PIC2_DATA);

    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore masks
    out(PIC1_DATA, a1);
    out(PIC2_DATA, a2);
}

void syscall(struct CPURegister cpu, __attribute__((unused)) struct InterruptStack info)
{
    if (cpu.eax == 0)
    {
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest *)cpu.ebx;
        *((int8_t *)cpu.ecx) = read(request, (uint32_t *)cpu.edx);
    }
    else if (cpu.eax == 1)
    {
        // TODO: Panggil File System Read Directory
        // Param:
        // cpu.ebx -> pointer request
        // cpu.ecx -> pointer retcode
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest *)cpu.ebx;
        *((int8_t *)cpu.ecx) = read_directory(request, (uint32_t *)cpu.edx);
    }
    else if (cpu.eax == 2)
    {
        // TODO: Panggil File System Write
        // Param:
        // cpu.ebx -> pointer request
        // cpu.ecx -> pointer retcode
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest *)cpu.ebx;
        *((int8_t *)cpu.ecx) = write(request);
    }
    else if (cpu.eax == 3)
    {
        // TODO: Panggil File System Delete
        // Param:
        // cpu.ebx -> pointer request
        // cpu.ecx -> pointer retcode
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest *)cpu.ebx;
        *((int8_t *)cpu.ecx) = delete (request);
    }
    else if (cpu.eax == 4)
    {
        reset_buffer();
        keyboard_state_activate();

        __asm__("sti"); // Due IRQ is disabled when main_interrupt_handler() called
        while (is_keyboard_blocking())
            ;
        char buf[KEYBOARD_BUFFER_SIZE] = {'\0'};
        get_keyboard_buffer(buf);
        memcpy((char *)cpu.ebx, buf, cpu.ecx);
    }
    else if (cpu.eax == 5)
    {
        // Modified puts() on kernel side
        puts_buff((char *)cpu.ebx, cpu.ecx, cpu.edx);
    }
    else if (cpu.eax == 6)
    {
        // read root directory
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest *)cpu.ebx;
        *((int8_t *)cpu.ecx) = read_root_directory(request);
    }
    else if (cpu.eax == 7){
        // rename dir
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest *)cpu.ebx;
        *((int8_t *)cpu.ecx) = rename_dir(request, (char *)cpu.edx);
    }
    else if (cpu.eax == 8){
        // move dir
        struct FAT32DriverRequest request = *(struct FAT32DriverRequest *)cpu.ebx;
        *((int8_t *)cpu.ecx) = move_dir(request, (uint32_t)cpu.edx);
    }

}

void activate_keyboard_interrupt(void)
{
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK ^ (1 << IRQ_KEYBOARD));
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void set_tss_kernel_current_stack(void)
{
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile("mov %%ebp, %0"
                     : "=r"(stack_ptr)
                     : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8;
}

void main_interrupt_handler(struct CPURegister cpu, uint32_t int_number, struct InterruptStack info)
{
    switch (int_number)
    {
    case PIC1_OFFSET + IRQ_KEYBOARD:
        keyboard_isr();
        break;
    case 0x30:
        syscall(cpu, info);
        break;
    }
}
