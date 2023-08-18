#include <alloca.h>
extern int g_val;
extern char *g_buf;

void test(void)
{
	g_buf = alloca(g_val);
}