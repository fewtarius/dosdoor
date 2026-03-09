/*
 * stubs.c - Stub implementations for stripped subsystems
 * dosdoor: stripped dosemu fork for BBS door games
 *
 * Provides no-op stubs for mouse, sound, DMA, debugger,
 * networking, and other subsystems not needed for
 * text-mode door game operation.
 *
 * Video/VGA core is provided by env/video/ (text-mode subset).
 * Stubs here cover removed video files: vc.c, vga.c, console.c,
 * hgc.c, dualmon.c, and their platform-specific symbols.
 */

#include "config.h"
#include "emu.h"
#include <stdarg.h>
#include "mhpdbg.h"
#include "video.h"
#include "disks.h"
#include "priv.h"
#include "keyboard.h"
#include "timers.h"

/*
 * Debugger stubs - debugger subsystem removed
 * Signatures must match mhpdbg.h declarations
 */
int vmhp_log_intercept(int flags, const char *fmt, va_list args) { return 0; }
void mhp_intercept_log(char *flags, int temporary) {}
void mhp_intercept(char *msg, char *logflags) {}
int mhp_getaxlist_value(int val, int mask) { return val; }
void mhp_printf(const char *fmt, ...) {}
unsigned int mhp_debug(unsigned int reason, unsigned int parm1, unsigned int parm2) { return 0; }
void mhp_close(void) {}
void mhp_exit_intercept(int errcode) {}
#ifndef USE_MHPDBG
int dpmi_mhp_get_selector_size(int sel) { return 0; }
#endif
#ifdef __APPLE__
void gdb_debug(void) {}
#endif

/*
 * Mouse stubs - mouse subsystem removed
 */
void dosemu_mouse_init(void) {}
void dosemu_mouse_reset(void) {}
void dosemu_mouse_close(void) {}
void mouse_helper(struct vm86_regs *regs) {}
int mouse_int(void) { return 0; }
void mouse_post_boot(void) {}
void mouse_ps2bios(void) {}
void mouse_keyboard(Boolean make, t_keysym key) {}
void mouse_curtick(void) {}
void mouse_move_absolute(int x, int y, int w, int h) {}
void mouse_move_buttons(int lbm, int mbm, int rbm) {}
void do_mouse_irq(void) {}
void register_mouse_client(struct mouse_client *mouse) {}
void freeze_mouse(void) {}
void unfreeze_mouse(void) {}

/*
 * Speaker stubs - speaker subsystem removed
 */
void register_speaker(void *s) {}
void speaker_on(unsigned short freq) {}
void speaker_off(void) {}
void speaker_pause(void) {}
void speaker_resume(void) {}
void console_speaker_on(void *gp, unsigned ms, unsigned short period) {
    (void)gp; (void)ms; (void)period;
}
void console_speaker_off(void *gp) { (void)gp; }

/*
 * DMA stubs - DMA subsystem removed
 */
struct dma_controller_struct { int dummy; };
struct dma_controller_struct dma_controller[2] = {{0}, {0}};
void dma_init(void) {}
void dma_reset(void) {}

/*
 * Networking stubs - networking subsystem removed
 */
void pkt_priv_init(void) {}

/*
 * CDROM stubs - CDROM subsystem removed
 */
void cdrom_helper(unsigned char *a, unsigned char *b) {}
char *Path_cdrom[26] = {0};

/*
 * MSCDEX stub - CDROM extension removed
 */
int mscdex(void) { return 0; }
void register_cdrom(int drive, int device) { (void)drive; (void)device; }
void unregister_cdrom(int drive) { (void)drive; }
int get_volume_label_cdrom(int drive, char *name) { (void)drive; (void)name; return 0; }

/*
 * Disassembly stub
 */
int dis_8086(void) { return 0; }

/*
 * Video stubs for removed hardware-specific files.
 * Core VGA emulator is in env/video/ (video.c, vgaemu.c, text.c, etc.)
 * These stubs replace: vc.c, vga.c, console.c, hgc.c, dualmon.c
 */

/* From vc.c - virtual console management (Linux console specific) */
int on_console(void) { return 0; }
void vt_activate(int con_num) { (void)con_num; }
int dos_has_vt = 1;
void set_linux_video(void) {}
void init_get_video_ram(int waitflag) { (void)waitflag; }
void get_video_ram(int waitflag) { (void)waitflag; }
void put_video_ram(void) {}
void set_process_control(void) {}

/* From console.c */
struct video_system Video_console = { 0 };
void clear_console_video(void) {}

/* From hgc.c */
struct video_system Video_hgc = { 0 };

/* From dualmon.c */
struct video_system *Video_default = NULL;
void init_dualmon(void) {}

/* From vga.c - hardware VGA access */
struct video_system Video_graphics = { 0 };
void save_vga_state(struct video_save_struct *save_regs) { (void)save_regs; }
void restore_vga_state(struct video_save_struct *save_regs) { (void)save_regs; }

/*
 * Privilege management stubs
 */
void close_kmem(void) {}
void open_kmem(void) {}

#ifdef __APPLE__
int priv_drop(void) { return 0; }
int real_leave_priv_setting(int *p) { (void)p; return 0; }
uid_t get_cur_euid(void) { return 0; }
gid_t get_cur_egid(void) { return 0; }
uid_t get_orig_euid(void) { return 0; }
uid_t get_orig_uid(void) { return 0; }
#endif

/*
 * Hardware RAM stubs - provided by mapping.c on Linux
 */
#ifdef __APPLE__
void get_hardware_ram(void) {}
void list_hardware_ram(void (*print_func)(char *, ...)) { (void)print_func; }
void map_hardware_ram(void) {}
void register_hardware_ram(int type, unsigned int base, unsigned int size) {
    (void)type; (void)base; (void)size;
}
#endif

/*
 * Console/VT stubs
 */
#ifdef __APPLE__
unsigned short detach(void) { return 0; }
#endif

/*
 * Disk stubs - provided by disks.c on Linux
 */
#ifdef __APPLE__
void disk_open(struct disk *dp) { (void)dp; }
#endif

/*
 * VESA BIOS binary blob stubs - vesabios.S and vesabios_pm.S removed.
 * Door games don't use VESA graphics modes.
 * These are declared as functions in vgaemu.h but are really just
 * labels pointing to memory in the original ASM blob.
 */
void vgaemu_bios_start(void) {}
void vgaemu_bios_end(void) {}
void vgaemu_bios_prod_name(void) {}
void vgaemu_bios_pm_interface(void) {}
void vgaemu_bios_pm_interface_end(void) {}
void vgaemu_bios_win_func(void) {}

/*
 * Video port I/O stubs - hardware VGA ports not needed
 */
void video_port_in(int port, unsigned char *val) { *val = 0; }
void video_port_out(int port, unsigned char val) { (void)port; (void)val; }

/*
 * Screen page stub - from vc.c
 */
void set_vc_screen_page(int page) { (void)page; }


/*
 * Joystick stubs - joystick subsystem removed
 */
void joy_init(void) {}
void joy_term(void) {}

/*
 * PCI stubs - PCI bus emulation removed
 */
struct pci_funcs;
struct pci_funcs *pciConfigType = 0;
int pci_setup(void) { return 0; }
void pci_bios(void) {}
struct pci_funcs *pci_check_conf(void) { return 0; }

/*
 * VESA/SVGA stubs - vesa.c removed
 */
void vbe_init(void *vedt) { (void)vedt; }
void do_vesa_int(void) {}
int vbe_scan_length(unsigned sub_func, unsigned scan_len) { (void)sub_func; (void)scan_len; return 0; }
int vbe_display_start(unsigned sub_func, unsigned x, unsigned y) { (void)sub_func; (void)x; (void)y; return 0; }
int vbe_dac_format(unsigned sub_func, unsigned bits) { (void)sub_func; (void)bits; return 0; }
int vbe_palette_data(unsigned sub_func, unsigned len, unsigned first, unsigned char *buffer) { (void)sub_func; (void)len; (void)first; (void)buffer; return 0; }
int vbe_pm_interface(unsigned sub_func) { (void)sub_func; return 0; }
int vbe_power_state(unsigned sub_func, unsigned state) { (void)sub_func; (void)state; return 0; }

/*
 * LPT/Printer stubs - parallel port removed
 */
#include "lpt.h"
struct printer lpt[NUM_PRINTERS] = {{0}};

int printer_open(int prnum) { (void)prnum; return -1; }
int printer_close(int prnum) { (void)prnum; return 0; }
int printer_flush(int prnum) { (void)prnum; return 0; }
int printer_write(int prnum, int outchar) { (void)prnum; (void)outchar; return 0; }
void printer_init(void) {}
void printer_mem_setup(void) {}
int printer_tick(unsigned long val) { (void)val; return 0; }
void printer_config(int prnum, struct printer *pptr) { (void)prnum; (void)pptr; }
void printer_print_config(int prnum, void (*print)(char *, ...)) { (void)prnum; (void)print; }
void close_all_printers(void) {}

/*
 * INT 17h BIOS printer service stub - int17.c removed
 */
int int17(void) { return 0; }

/*
 * Hercules stubs - hercemu.c removed
 */
void Herc_init(void) {}
void Herc_set_cfg_switch(unsigned char val) { (void)val; }
void Herc_set_mode_ctrl(unsigned char val) { (void)val; }
unsigned char Herc_get_mode_ctrl(void) { return 0; }

/*
 * Install/Welcome stubs - install.c removed (no first-time wizard)
 */
int unix_e_welcome = 0;
void install_dos(int post_boot) { (void)post_boot; }
void show_welcome_screen(void) {}

/*
 * Userhook stubs - userhook.c removed (frontend control)
 */
int uhook_fdin = -1;
void uhook_input(void) {}
void uhook_poll(void) {}
void init_uhook(char *pipes) { (void)pipes; }
void close_uhook(void) {}

/* dump_keytable - keytable dump for config debug.
 * The full implementation is in kbd_unicode/keymaps.c.
 * This weak stub satisfies the linker when kbd_unicode is not linked. */
#include <stdio.h>
struct keytable_entry;
__attribute__((weak))
void dump_keytable(FILE *f, struct keytable_entry *kt) {
    (void)f; (void)kt;
    fprintf(f, "(keytable dump not available)\n");
}
