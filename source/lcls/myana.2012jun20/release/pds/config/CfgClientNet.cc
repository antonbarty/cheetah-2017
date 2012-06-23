#include "CfgClientNet.hh"

#include "CfgPorts.hh"
#include "CfgRequest.hh"
#include "pds/utility/OutletWireHeader.hh"

#include <string.h>
#include <unistd.h>
#include <errno.h>

using namespace Pds;

CfgClientNet::CfgClientNet( const Src& src,
			    unsigned platform ) :
  _src (src),
  _platform (platform)
{
}

void CfgClientNet::setDbName(const char* dbname) 
{
  memcpy(_dbName,dbname,sizeof(_dbName));
}

//
//  Returns length of configuration record
//
int CfgClientNet::fetch(const Transition& tr,
			const TypeId&     id,
			char*             dst)
{
  int length;
  OutletWireHeader* header;

  // Request a configuration server (src,platform)
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  CfgRequest svc(_src, id, tr);

  Ins ins(CfgPorts::ins(_platform));
  sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = htonl(ins.address());
  saddr.sin_port        = htons(ins.portId());
  if (::sendto(sockfd, &svc, sizeof(svc), 0, (const sockaddr*)&saddr, sizeof(saddr)) < 0) {
    length = -errno;
    goto err;
  }

  char headerb[sizeof(OutletWireHeader)];
  header = reinterpret_cast<OutletWireHeader*>(headerb);
  iovec iov[2];
  iov[0].iov_base = header;
  iov[0].iov_len  = sizeof(*header);
  iov[1].iov_base = dst;
  iov[1].iov_len  = 32*1024;
  struct msghdr msg;
  msg.msg_name    = 0;
  msg.msg_namelen = 0;
  msg.msg_iov    = iov;
  msg.msg_iovlen = 2;

  int nbytes;
  nbytes  = ::recvmsg(sockfd, &msg, 0);
  nbytes -= sizeof(*header);
  if (nbytes < 0) {
      length = -EINVAL;
      goto err;
    }

  length = header->offset+nbytes;
  while( length < header->length ) {
    iov[1].iov_base = dst + length;
    nbytes  = ::recvmsg(sockfd, &msg, 0);
    nbytes -= sizeof(*header);
    if (nbytes < 0 || header->offset != length) {
      length = -EINVAL;
      goto err;
    }
    length += nbytes;
  }
 err:
  ::close(sockfd);
  return length;
}

