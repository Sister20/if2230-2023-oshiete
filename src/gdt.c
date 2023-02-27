#include "lib-header/stdtype.h"
#include "lib-header/gdt.h"

/**
 * global_descriptor_table, predefined GDT.
 * Initial SegmentDescriptor already set properly according to GDT definition in Intel Manual & OSDev.
 * Table entry : [{Null Descriptor}, {Kernel Code}, {Kernel Data (variable, etc)}, ...].
 */
struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {
            // TODO : Implement
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
            // TODO : Implement
            .segment_low = 0xFFFF,
            .base_low = 0,
            .base_mid = 0,
            .type_bit = 0xA,
            .non_system = 1,
            .dpl = 0,
            .segment_present = 1,
            .limit = 0xF,
            .avl = 0,
            .code_segment = 1,
            .def_op_size = 1,
            .granularity = 1,
            .base_high = 0
        },
        {
            // TODO : Implement
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
        }
    }
};

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

