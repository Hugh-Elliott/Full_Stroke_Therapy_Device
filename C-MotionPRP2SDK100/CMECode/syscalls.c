/***********************************************************************/
/*                                                                     */
/*  SYSCALLS.C:  System Calls                                          */
/*                                                                     */
/***********************************************************************/


#include <stdlib.h>
#include <reent.h>
#include <sys/stat.h>
#include "PMDtypes.h"
#include "PMDsys.h"
#include "locations.h"

_ssize_t _read_r(
    struct _reent *r, 
    int file, 
    void *ptr, 
    size_t len)
{
	return 0;
}

_ssize_t _write_r (
    struct _reent *r, 
    int file, 
    const void *ptr, 
    size_t len)
{
	const char *p;
	
	p = (const char*) ptr;
	return PMDwrite(p, len);	
}


int _close_r(
    struct _reent *r, 
    int file)
{
	return 0;
}


_off_t _lseek_r(
    struct _reent *r, 
    int file, 
    _off_t ptr, 
    int dir)
{
	return (_off_t)0;	/*  Always indicate we are at file beginning.  */
}


int _fstat_r(
    struct _reent *r, 
    int file, 
    struct stat *st)
{
	/*  Always set as character device.				*/
	st->st_mode = S_IFCHR;
	/* assigned to strong type with implicit 	*/
	/* signed/unsigned conversion.  Required by 	*/
	/* newlib.					*/

	return 0;
}

int _isatty(int file)
{
	return 1;
}
int isatty(int file)
{
	return 1;
}

/**** Locally used variables. ****/
extern char end;		/* end is set in the linker command 	*/
				/* file and is the end of statically 	*/
				/* allocated data (thus start of heap).	*/

static char *heap_ptr = 0;	/* Points to current end of the heap.	*/

/************************** _sbrk_r *************************************
 * Support function. Adjusts end of heap to provide more memory to
 * memory allocator. 
 *
 *  struct _reent *r -- re-entrancy structure, used by newlib to
 *                      support multiple threads of operation.
 *  ptrdiff_t nbytes -- number of bytes to add.
 *                      Returns pointer to start of new heap area.
 *
 *  Note:  This implementation is not thread safe (despite taking a
 *         _reent structure as a parameter).
 *         Since _s_r is not used in the current implementation, 
 *         the following messages must be suppressed.
 */
void * _sbrk_r(
    struct _reent *_s_r, 
    ptrdiff_t nbytes)
{
	char  *base;		/*  errno should be set to  ENOMEM on error  */

	if (!heap_ptr) {	/*  Initialize if first time through.  */
		heap_ptr = &end;
	}
	base = heap_ptr;	/*  Point to end of heap.  */

	if (base > (char *)UM_ADDRESS_END)
	{
		base = (void *)(-1);
		_write_r (_s_r, 1, "Heap empty\n", 11);
	}	
	else
	{
		heap_ptr += nbytes;	/*  Increase heap.  */
	}

	return base;		/*  Return pointer to start of new heap area.  */
}

void _exit(int code)
{
	PMDTaskAbort(code);
	while(1); // shouldn't get here
}

int _getpid ( void )
{
	return 0;
}

int _kill ( struct _reent *ptr )
{
	/* return "not supported" */
//	ptr->_errno = ENOTSUP;
	return -1;
}

