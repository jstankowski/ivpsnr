/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xMatrix_IMPLEMENTATION
#include "xMatrix.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

//instantiation of most often types
template class xMatrix<4, 4, flt32>;
template class xMatrix<3, 4, flt32>;
template class xMatrix<3, 3, flt32>;
template class xMatrix<3, 1, flt32>;
template class xMatrix<4, 4, flt64>;
template class xMatrix<3, 4, flt64>;
template class xMatrix<3, 3, flt64>;
template class xMatrix<3, 1, flt64>;

template void xMatrix<4, 4, flt32>::setInvertion<4,4>(const xMatrix<4, 4, flt32>& Src);
template void xMatrix<3, 3, flt32>::setInvertion<3,3>(const xMatrix<3, 3, flt32>& Src);
template void xMatrix<4, 4, flt64>::setInvertion<4,4>(const xMatrix<4, 4, flt64>& Src);
template void xMatrix<3, 3, flt64>::setInvertion<3,3>(const xMatrix<3, 3, flt64>& Src);

//===============================================================================================================================================================================================================

} //end of namespace PMBB
