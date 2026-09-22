/* Mach-O C symbols have a leading underscore; ELF symbols do not. */
#ifdef __APPLE__
#define CPUFP_SYMBOL(name) _##name
#else
#define CPUFP_SYMBOL(name) name
#endif
