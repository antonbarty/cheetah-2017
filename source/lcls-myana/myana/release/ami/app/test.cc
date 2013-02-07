#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>

int main()
{
  iovec _iov[3];
  _iov[0].iov_base = (void*)0x8000000; _iov[0].iov_len = 0x10;
  _iov[1].iov_base = (void*)0x9000000; _iov[1].iov_len = 0x18000;
  _iov[2].iov_base = (void*)0xa000000; _iov[2].iov_len = 0x80;
  unsigned cnt = 2;

  unsigned payload = 0x10 + 0x18000 + 0x80;
  const unsigned CHUNKSIZE=0x7F00;
  if (payload>CHUNKSIZE) {

    unsigned offset  = 0;
    unsigned remaining = payload;
    unsigned chunk     = CHUNKSIZE;
    iovec* iov = _iov;
    do {
      for(unsigned i=0; i<=cnt; i++)
        printf(".iov[%d] %p/%x\n",i,_iov[i].iov_base,_iov[i].iov_len);

      int r = chunk;
      iovec* iiov = iov;
      unsigned s=0;
      while( r>0 )
        r -= (s=(++iiov)->iov_len);
      iiov->iov_len += r;

      iov->iov_base = (void*)0x1000000; iov->iov_len = 0x10;

      for(int i=0; i<=iiov-iov; i++)
        printf("iov[%d] %p/%x\n",i,iov[i].iov_base,iov[i].iov_len);

      iov        = iiov-1;
      iiov->iov_len   = -r;
      iiov->iov_base = (char*)iiov->iov_base+s+r;
      offset    += chunk;
      remaining -= chunk;
      if (remaining < chunk) chunk = remaining;

      printf("iov %p  offset %x  remaining %x  chunk %x\n",
	     iov,offset,remaining,chunk);

    } while(remaining);
  }
  return 0;
}
