#include "lib-header/stdtype.h"
#include "lib-header/gdt.h"

static struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            // TODO : Implement NULL DESCRIPTOR
            .segment_low = 0x00000000,
            .base_low =  0x0000,
            .base_mid =  0x00,
            .type_bit = 0x0,
            .non_system = 0,
            .dpl = 0,
            .segment_present = 0,
            .limit = 0x0,
            .avl = 0,
            .code_segment = 0,
            .def_op_size = 0,
            .granularity = 0,
            .base_high = 0x00
        },
        {
            // TODO : Implement KERNEL CODE DESCRIPTOR
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0xA,
            .non_system = 1,
            .dpl = 0,
            .segment_present = 1,
            .limit = 0xF,
            .avl = 0,
            .code_segment = 0,
            .def_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            // TODO : Implement KERNEL DATA DESCIPTOR
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0x2,
            .non_system = 1,
            .dpl = 0,
            .segment_present = 1,
            .limit = 0xF,
            .avl = 0,
            .code_segment = 0,
            .def_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {/* TODO: User Code Descriptor */
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0xA,
            .non_system = 1,
            .dpl = 0x3,
            .segment_present = 1,
            .limit = 0xF,
            .avl = 0,
            .code_segment = 0,
            .def_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {/* TODO: User Data Descriptor */
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0x2,
            .non_system = 1,
            .dpl = 0x3,
            .segment_present = 1,
            .limit = 0xF,
            .avl = 0,
            .code_segment = 0,
            .def_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            .limit              = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .non_system        = 0,    // S bit
            .type_bit          = 0x9,
            .dpl         = 0,    // DPL
            .segment_present        = 1,    // P bit
            .def_op_size        = 1,    // D/B bit
            .code_segment         = 0,    // L bit
            .granularity       = 0,    // G bit
        },
        {0}
    }
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}

/**
 * _gdt_gdtr, predefined system GDTR. 
 * GDT pointed by this variable is already set to point global_descriptor_table above.
 * From: https://wiki.osdev.org/Global_Descriptor_Table, GDTR.size is GDT size minus 1.
 */
struct GDTR _gdt_gdtr = {
    // TODO : Implement, this GDTR will point to global_descriptor_table. 
    //        Use sizeof operator
    .size = sizeof(struct GlobalDescriptorTable) - 1,
    .address = &global_descriptor_table
};

