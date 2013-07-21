

#define ERROR(...) cheetahError(__FILE__, __LINE__, __VA_ARGS__)

void static cheetahError(const char *filename, int line, const char *format, ...){
	va_list ap;
	va_start(ap,format);
	fprintf(stderr,"CHEETAH-ERROR in %s:%d: ",filename,line);
	vfprintf(stderr,format,ap);
	va_end(ap);
	puts("");
	abort();
}


