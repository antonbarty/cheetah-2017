#include "ami/qt/AbsClient.hh"

using namespace Ami::Qt;

AbsClient::AbsClient(QWidget*            parent,
		     const Pds::DetInfo& src, 
		     unsigned            channel) :
  QtTopWidget(parent,src,channel) {}

AbsClient::~AbsClient() {}
