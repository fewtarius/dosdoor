/* userhook.h - stub header, userhook subsystem removed */
#ifndef DOSDOOR_USERHOOK_H
#define DOSDOOR_USERHOOK_H

extern int uhook_fdin;
static inline void uhook_input(void) {}
static inline void uhook_poll(void) {}
static inline void init_uhook(char *pipes) { (void)pipes; }
static inline void close_uhook(void) {}

#endif
