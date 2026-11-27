/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xBfrPic_IMPLEMENTATION
#include "xBfrPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xBfrPlane - instantiation for base types
//===============================================================================================================================================================================================================

template class xBfrPicP<uint8 ,3>;
template class xBfrPicP< int8 ,3>;
template class xBfrPicP<uint16,3>;
template class xBfrPicP< int16,3>;
template class xBfrPicP<uint32,3>;
template class xBfrPicP< int32,3>;
template class xBfrPicP<uint64,3>;
template class xBfrPicP< int64,3>;
template class xBfrPicP< flt32,3>;
template class xBfrPicP< flt64,3>;

//===============================================================================================================================================================================================================

} //end of namespace PMBB