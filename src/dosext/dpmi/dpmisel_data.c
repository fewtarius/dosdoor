/*
 * dpmisel_data.c - DPMI selector code area for non-x86 platforms
 * These are HLT opcodes (0xF4) that trigger the CPU emulator's
 * DPMI handling when executed.
 */

/* HLT opcode */
#define HLT 0xF4
/* LRET opcode */
#define LRET 0xCB

/* The DPMI direct transfer functions are not used on non-x86
   (software CPU emulation handles context switching) */
void DPMI_direct_transfer(void) {}
void DPMI_direct_transfer_end(void) {}
void DPMI_indirect_transfer(void) {}
void DPMI_iret(void) {}

/* DPMI selector code area - a series of HLT instructions that
   the CPU emulator intercepts */
unsigned char DPMI_sel_code_start[] = {
    /* 0x0000: DPMI_raw_mode_switch_pm */
    HLT,
    /* 0x0001: DPMI_save_restore_pm */
    HLT, LRET,
    /* 0x0003: DPMI_API_extension */
    HLT, LRET,
    /* 0x0005: DPMI_return_from_pm */
    HLT,
    /* 0x0006: DPMI_return_from_exception */
    HLT,
    /* 0x0007: DPMI_return_from_rm_callback */
    HLT,
    /* 0x0008: DPMI_exception (32 HLTs) */
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    /* 0x0028: DPMI_interrupt (256 HLTs) */
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,HLT,
    /* 0x0128: DPMI_return_from_ext_exception */
    HLT,
    /* 0x0129: DPMI_return_from_int_1c */
    HLT,
    /* 0x012A: DPMI_return_from_int_23 */
    HLT,
    /* 0x012B: DPMI_return_from_int_24 */
    HLT,
    /* 0x012C: DPMI_return_from_RSPcall */
    HLT,
    /* 0x012D: MSDOS_return_from_pm */
    HLT,
    /* 0x012E: MSDOS_XMS_call */
    HLT, LRET,
    /* 0x0130: DPMI_VXD_VMM */
    HLT, LRET,
    /* 0x0132: DPMI_VXD_PageFile */
    HLT, LRET,
    /* 0x0134: DPMI_VXD_Reboot */
    HLT, LRET,
    /* 0x0136: DPMI_VXD_VDD */
    HLT, LRET,
    /* 0x0138: DPMI_VXD_VMD */
    HLT, LRET,
    /* 0x013A: DPMI_VXD_VXDLDR */
    HLT, LRET,
    /* 0x013C: DPMI_VXD_SHELL */
    HLT, LRET,
    /* 0x013E: DPMI_VXD_VCD */
    HLT, LRET,
    /* 0x0140: DPMI_VXD_VTD */
    HLT, LRET,
    /* 0x0142: DPMI_VXD_CONFIGMG */
    HLT, LRET,
    /* 0x0144: DPMI_VXD_ENABLE */
    HLT, LRET,
    /* 0x0146: DPMI_VXD_APM */
    HLT, LRET,
    /* 0x0148: DPMI_VXD_VTDAPI */
    HLT, LRET,
};

/* Symbol pointers into the code area */
unsigned char *DPMI_raw_mode_switch_pm = &DPMI_sel_code_start[0x00];
unsigned char *DPMI_save_restore_pm = &DPMI_sel_code_start[0x01];
unsigned char *DPMI_API_extension = &DPMI_sel_code_start[0x03];
unsigned char *DPMI_return_from_pm = &DPMI_sel_code_start[0x05];
unsigned char *DPMI_return_from_exception = &DPMI_sel_code_start[0x06];
unsigned char *DPMI_return_from_rm_callback = &DPMI_sel_code_start[0x07];
unsigned char *DPMI_exception = &DPMI_sel_code_start[0x08];
unsigned char *DPMI_interrupt = &DPMI_sel_code_start[0x28];
unsigned char *DPMI_return_from_ext_exception = &DPMI_sel_code_start[0x128];
unsigned char *DPMI_return_from_int_1c = &DPMI_sel_code_start[0x129];
unsigned char *DPMI_return_from_int_23 = &DPMI_sel_code_start[0x12A];
unsigned char *DPMI_return_from_int_24 = &DPMI_sel_code_start[0x12B];
unsigned char *DPMI_return_from_RSPcall = &DPMI_sel_code_start[0x12C];
unsigned char *MSDOS_return_from_pm = &DPMI_sel_code_start[0x12D];
unsigned char *MSDOS_rrm_start = &DPMI_sel_code_start[0x12D];
unsigned char *MSDOS_rrm_end = &DPMI_sel_code_start[0x12E];
unsigned char *MSDOS_spm_start = &DPMI_sel_code_start[0x12E];
unsigned char *MSDOS_XMS_call = &DPMI_sel_code_start[0x12E];
unsigned char *MSDOS_spm_end = &DPMI_sel_code_start[0x130];
unsigned char *DPMI_VXD_start = &DPMI_sel_code_start[0x130];
unsigned char *DPMI_VXD_VMM = &DPMI_sel_code_start[0x130];
unsigned char *DPMI_VXD_PageFile = &DPMI_sel_code_start[0x132];
unsigned char *DPMI_VXD_Reboot = &DPMI_sel_code_start[0x134];
unsigned char *DPMI_VXD_VDD = &DPMI_sel_code_start[0x136];
unsigned char *DPMI_VXD_VMD = &DPMI_sel_code_start[0x138];
unsigned char *DPMI_VXD_VXDLDR = &DPMI_sel_code_start[0x13A];
unsigned char *DPMI_VXD_SHELL = &DPMI_sel_code_start[0x13C];
unsigned char *DPMI_VXD_VCD = &DPMI_sel_code_start[0x13E];
unsigned char *DPMI_VXD_VTD = &DPMI_sel_code_start[0x140];
unsigned char *DPMI_VXD_CONFIGMG = &DPMI_sel_code_start[0x142];
unsigned char *DPMI_VXD_ENABLE = &DPMI_sel_code_start[0x144];
unsigned char *DPMI_VXD_APM = &DPMI_sel_code_start[0x146];
unsigned char *DPMI_VXD_VTDAPI = &DPMI_sel_code_start[0x148];
unsigned char *DPMI_VXD_end = &DPMI_sel_code_start[0x14A];
unsigned char *DPMI_sel_code_end = &DPMI_sel_code_start[0x14A];
