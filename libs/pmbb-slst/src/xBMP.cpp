#include "xBMP.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

static_assert(sizeof(xBitmapFileHeader) == xBitmapFileHeader::c_HeaderLength);
static_assert(sizeof(xBitmapInfoHeader) == xBitmapInfoHeader::c_HeaderLength);

//===============================================================================================================================================================================================================

bool xBitmapFileHeader::Read(xStream* Stream)
{
  if(!Stream->canRead()) { return false; }
  bool Result = Stream->read(this, c_HeaderLength);
  return Result;
}
bool xBitmapFileHeader::Write(xStream* Stream)
{
  if(!Stream->canWrite()) { return false; }
  bool Result = Stream->write(this, c_HeaderLength);
  return Result;
}
bool xBitmapFileHeader::Validate() const
{
  bool Result = true;
  if(m_Type != 0x4D42) { Result = false; }
  return Result;
}

//===============================================================================================================================================================================================================

bool xBitmapInfoHeader::Read(xStream* Stream)
{
  if(!Stream->canRead()) { return false; }
  bool Result = Stream->read(this, c_HeaderLength);
  return Result;
}
bool xBitmapInfoHeader::Write(xStream* Stream)
{
  if(!Stream->canWrite()) { return false; }
  bool Result = Stream->write(this, c_HeaderLength);
  return Result;
}
bool xBitmapInfoHeader::Validate() const
{
  bool Result = true;
  if(m_HeaderSize != c_HeaderLength) { Result = false; }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
