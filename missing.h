#ifndef MISSING_H
#define MISSING_H 1

#if (!HAVE_POPEN)
extern FILE *popen(const char *, const char*);
extern int *pclose(FILE *fp);
#endif

#if (!HAVE_FILENO)
extern int fileno(FILE *fp); /* should be in stdio ... */
#endif

#if (!HAVE_VSNPRINTF)
#define HAVE_SNPRINTF 0
extern int vsnprintf(char *buff,size_t s, const char *format, va_list ap);
#endif

#if (!HAVE_SNPRINTF)
extern int snprintf(char *buff,size_t s, const char *format, ...);
#endif

#if (!HAVE_FTRUNCATE)
#define off_t size_t
extern int ftruncate(int fd, off_t pos);
#undef off_t 
#endif

#endif /* MISSING_H */
