/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"
#include "xString.h"
#include "xMatrixUtils.h"
#include "xHelpersFLT.h"
#include <array>
#include <vector>

/*
R = ROWS = Y = H
C = COLS = X = W
M[Y][X]
*/

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

//template<uint32 Rows, uint32 Cols, typename MatrixType>
template<uint32 R, uint32 C, typename T> class xMatrix
{
protected:
#if X_SIMD_CAN_USE_AVX
  alignas(alignof(__m256)) std::array<std::array<T, C>, R> m_E;
#elif X_SIMD_CAN_USE_SSE
  alignas(alignof(__m128)) std::array<std::array<T, C>, R> m_E;
#else
  std::array<std::array<T, C>, R> m_E;
#endif

public:
  xMatrix (                                             ) {               } //default constructor
//xMatrix (const T Value                                ) { fill(Value);  } //parameterized constructor
  xMatrix (const xMatrix<R, C, T>&                Matrix) { copy(Matrix); } //copy constructor  
  xMatrix (const std::array<std::array<T, C>, R>& ArrArr) { m_E = ArrArr; } //construcor (Array of Arrays)
  template <uint32 RR = R, uint32 CC = C> xMatrix(const std::array<T, R>& Vec, typename std::enable_if<CC == 1, void>::type* = 0) { memcpy(m_E[0].data(), Vec.data(), getArea() * sizeof(T)); } //construcor (Horizontal vector C=1)
  template <uint32 RR = R, uint32 CC = C> xMatrix(const std::array<T, C>& Vec, typename std::enable_if<RR == 1, void>::type* = 0) { memcpy(m_E[0].data(), Vec.data(), getArea() * sizeof(T)); } //construcor (Vertical   vector R=1)
  template <uint32 RR = R, uint32 CC = C> xMatrix(std::initializer_list<T> L, typename std::enable_if<CC == 1, void>::type* = 0) { uint32 i = 0; for(auto v : L) { m_E[i][0] = v; i++; }; }
  template <uint32 RR = R, uint32 CC = C> xMatrix(std::initializer_list<T> L, typename std::enable_if<RR == 1, void>::type* = 0) { uint32 i = 0; for(auto v : L) { m_E[0][i] = v; i++; }; }

  //copy assignment operator
  xMatrix<R, C, T>& operator= (const xMatrix<R, C, T>& Src) { copy(Src); return *this; } 

  //convertion operator
  template<typename OtherType> explicit operator xMatrix<R, C, OtherType>() const { return getConverted<OtherType>(); }

  //utils
  inline void copy (const xMatrix<R, C, T>& Src) { memcpy(m_E[0].data(), Src.m_E[0].data(), getArea() * sizeof(T)); }
         void load (const T* Src, uint32 SrcRows, uint32 SrcCols, uint32 SrcStride);
         void store(T* Dst, uint32 DstRows, uint32 DstCols, uint32 DstStride) const;
  inline void zero (             ) { memset(m_E[0].data(), 0, getArea() * sizeof(T)); }
  inline void fill (const T Value) { xMemsetX<T>(m_E[0].data(), Value, getArea()); }
  
  //matrix operations - type convertion
  template <typename OtherType> void                     setConverted(const xMatrix<R, C, OtherType>& Src);
  template <typename OtherType> xMatrix<R, C, OtherType> getConverted(                                   ) const;

  //matrix operations - compare
  bool isEqual             (const xMatrix<R, C, T>& M                                                 ) const;
  bool isApproximatelyEqual(const xMatrix<R, C, T>& M, T Tolerance = std::numeric_limits<T>::epsilon()) const;

  //matrix operations - round close values
  inline void              setRoundValuesCloseToZeroOrOne(const xMatrix<R, C, T>& M, T Tolerance = std::numeric_limits<T>::epsilon());
  inline xMatrix<R, C, T>  getRoundValuesCloseToZeroOrOne(                           T Tolerance = std::numeric_limits<T>::epsilon()) const;

  //matrix operations - identity - only NxN matrix
  template <uint32 RR = R, uint32 CC = C> typename std::enable_if<RR == CC, void>::type setIdentity() { zero(); for(uint32 i = 0; i < R; i++) {  m_E[i][i] = (T)1; } }
  bool isIdentity       (                                                     ) const;
  bool isCloseToIdentity(T ToleranceOffset = std::numeric_limits<T>::epsilon()) const;

  //matrix operations - invert - only NxN matrix
  template <uint32 RR = R, uint32 CC = C> typename std::enable_if<RR == CC, void>::type               setInvertion(const xMatrix<R, C, T>& Src);
  template <uint32 RR = R, uint32 CC = C> typename std::enable_if<RR == CC, xMatrix<R, C, T>>::type   getInvertion() { xMatrix<R, C, T> M; M.setInvertion(*this); return M; }
  template <uint32 RR = R, uint32 CC = C> typename std::enable_if<RR == CC, xMatrix<R, C, T>&>::type& modInvert   () { setInvertion(*this); return *this; }

  //matrix operations - transpose - only NxN matrix or MxN-->NxM
  template<uint32 RR = R, uint32 CC = C> inline typename std::enable_if<(RR == C && CC == R), void>::type    setTransposed   (const xMatrix<RR, CC, T>& Src) { for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { m_E[i][j] = Src[j][i]; } } }
  inline xMatrix<C, R, T>                                                                            getTransposition() const { xMatrix<C, R, T> M; M.setTransposed(*this); return M; }
  inline xMatrix<C, R, T>                                                                            getTransposition()       { xMatrix<C, R, T> M; M.setTransposed(*this); return M; }
  template<uint32 RR, uint32 CC> inline typename std::enable_if<(RR == CC), xMatrix<R, C, T>&>::type modTranspose    ()       { xMatrix<R, C, T> M; M.setTransposed(*this); *this = M; return M; }

  //matrix operations - add, sub
  inline xMatrix<R, C, T>& operator  += (const xMatrix<R, C, T>& Mat)       { for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { m_E[h][w] += Mat[h][w]; } } return *this; }
  inline xMatrix<R, C, T>& operator  -= (const xMatrix<R, C, T>& Mat)       { for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { m_E[h][w] -= Mat[h][w]; } } return *this; }
  inline xMatrix<R, C, T>  operator  +  (const xMatrix<R, C, T>& Mat) const { xMatrix<R, C, T> M; for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { M[h][w] = m_E[h][w] + Mat[h][w]; } } return M; }
  inline xMatrix<R, C, T>  operator  -  (const xMatrix<R, C, T>& Mat) const { xMatrix<R, C, T> M; for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { M[h][w] = m_E[h][w] - Mat[h][w]; } } return M; }
  
  //matrix operations - mul - only RxC=RxS*SxC 
  template<uint32 S> static inline xMatrix<R, C, T> MultiplyMatrixByMatrix(const xMatrix<R, S, T>& A, const xMatrix<S, C, T>& B) { xMatrix<R, C, T> D; D.setMultiplyMatrixByMatrix(A, B); return D; } //calculate C = A*B
  template<uint32 S> inline void              setMultiplyMatrixByMatrix   (const xMatrix<R, S, T>& A, const xMatrix<S, C, T>& B); //calculate *this = A*B
  template<uint32 S> inline xMatrix<R, C, T>  getMultiplyByMatrix         (                           const xMatrix<S, C, T>& B) const { xMatrix<R, C, T> D; D.setMultiplyMatrixByMatrix(*this, B); return D; } //calculate return A*B
  template<uint32 S> inline xMatrix<R, C, T>& modMultiplyByMatrix         (                           const xMatrix<S, C, T>& B)       { xMatrix<R, C, T> D; D.setMultiplyMatrixByMatrix(*this, B); return D; }  //calculate *this = *this * B
  
  //scalar operations - add, sub
  inline xMatrix<R, C, T>& operator  += (const T& Scalar)       { for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { m_E[h][w] += Scalar; } } return *this; }
  inline xMatrix<R, C, T>& operator  -= (const T& Scalar)       { for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { m_E[h][w] -= Scalar; } } return *this; }
  inline xMatrix<R, C, T>  operator  +  (const T& Scalar) const { xMatrix<R, C, T> M; for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { M[h][w] = m_E[h][w] + Scalar; } } return M; }
  inline xMatrix<R, C, T>  operator  -  (const T& Scalar) const { xMatrix<R, C, T> M; for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { M[h][w] = m_E[h][w] - Scalar; } } return M; }
  
  //scalar operations - mul
  static inline xMatrix<R, C, T> MultiplyMatixByScalar(const xMatrix<R, C, T>& M, const T S) { xMatrix<R, C, T> D; D.setMultiplyMatrixByScalar(M, S); return D; }
  inline void              setMultiplyMatrixByScalar(const xMatrix<R, C, T>& M, const T S) { for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { m_E[i][j] = M[i][j] * S; } } }
  inline xMatrix<R, C, T>  getMultiplyByScalar      (const T S) const { xMatrix<R, C, T> D; for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { D[i][j] = m_E[i][j] * S; } }; return D; }
  inline xMatrix<R, C, T>& modMultiplyByScalar      (const T S) { for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { m_E[i][j] = m_E[i][j] * S; } } return *this; }

  //scalar operation - div
  static inline xMatrix<R, C, T> DivideMatixByScalar(const xMatrix<R, C, T>& M, const T S) { xMatrix<R, C, T> D; D.setMultiplyMatrixByScalar(M, S); return D; }
  inline void              setDivideMatrixByScalar(const xMatrix<R, C, T>& M, const T S) { for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { m_E[i][j] = M[i][j] / S; } } }
  inline xMatrix<R, C, T>  getDivideByScalar      (const T S) const { xMatrix<R, C, T> D; for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { D[i][j] = m_E[i][j] / S; } }; return D; }
  inline xMatrix<R, C, T>& modDivideByScalar      (const T S) { for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { m_E[i][j] = m_E[i][j] / S; } } return *this; }
  
  //element access - "at(y, x)" or "at(y)" or "at(x)"
  inline        T& at (uint32 y, uint32 x)       { return m_E.at(y).at(x); }
  inline const  T& at (uint32 y, uint32 x) const { return m_E.at(y).at(x); }
  template <uint32 RR = R, uint32 CC = C> inline typename std::enable_if<CC == 1,       T&>::type at(int32 y)       { return m_E.at(y).at(0); }
  template <uint32 RR = R, uint32 CC = C> inline typename std::enable_if<CC == 1, const T&>::type at(int32 y) const { return m_E.at(y).at(0); }
  template <uint32 RR = R, uint32 CC = C> inline typename std::enable_if<RR == 1,       T&>::type at(int32 x)       { return m_E.at(0).at(x); }
  template <uint32 RR = R, uint32 CC = C> inline typename std::enable_if<RR == 1, const T&>::type at(int32 x) const { return m_E.at(0).at(x); }

  //element access - [y][x]
  //template <uint32 RR = R, uint32 CC = C> inline typename std::enable_if<RR >= 2 && CC >= 2,       std::array<T, C>&>::type  operator[] (int32 y)       { return m_E[y]; }
  //template <uint32 RR = R, uint32 CC = C> inline typename std::enable_if<RR >= 2 && CC >= 2, const std::array<T, C>&>::type  operator[] (int32 y) const { return m_E[y]; }
  inline        std::array<T, C>& operator[] (int32 y)       { return m_E[y]; }
  inline const  std::array<T, C>& operator[] (int32 y) const { return m_E[y]; }  

  //element access - pointer
  inline        T* getPtr()       { return m_E[0].data(); }
  inline const  T* getPtr() const { return m_E[0].data(); }

  //info
  constexpr uint32 getRows() const { return R;     }
  constexpr uint32 getCols() const { return C;     }
  constexpr uint32 getArea() const { return R * C; }

  //print
  void          display(const std::string& Prefix) const { fmt::print("{}", print(Prefix, Prefix, "  ")); }
  std::string   print  (const std::string& Prefix1st, const std::string& PrefixNth, const std::string& Separator) const;
  std::string   print  (const std::string& Prefix1st, const std::string& PrefixNth, const std::string& Separator, uint32 NumRows, uint32 NumCols) const;

  //experimetal operations
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type InvertLowerTriangular(xMatrix<R, C, T>& InvL, const xMatrix<R, C, T>& L);
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type InvertUpperTriangular(xMatrix<R, C, T>& InvU, const xMatrix<R, C, T>& U);

  //Cholesky decomposition - decomposition of a Hermitian, positive-definite matrix into the product of a lower triangular matrix
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type DecomposeCholesky(xMatrix<R, C, T>& L   , const xMatrix<R, C, T>& A);
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type InvertCholesky   (xMatrix<R, C, T>& InvA, const xMatrix<R, C, T>& A);

  //LU decomposition
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type DecomposeLU(xMatrix<R, C, T>& L, xMatrix<R, C, T>& U, const xMatrix<R, C, T>& A);
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type InvertLU   (xMatrix<R, C, T>& InvA, const xMatrix<R, C, T>& A);

  //QR decomposition
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type DecomposeQR(xMatrix<R, C, T>& Q, xMatrix<R, C, T>& U, const xMatrix<R, C, T>& A);
  template <uint32 RR = R, uint32 CC = C> static typename std::enable_if<RR == CC, bool>::type InvertQR   (xMatrix<R, C, T>& InvA, const xMatrix<R, C, T>& A);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// utils
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> void xMatrix<R, C, T>::load(const T* Src, uint32 SrcRows, uint32 SrcCols, uint32 SrcStride)
{
  const uint32 NumRows = xMin(R, SrcRows);
  const uint32 NumCols = xMin(C, SrcCols);
  for(uint32 h = 0; h < NumRows; h++)
  { 
    for(uint32 w = 0; w < NumCols; w++) { m_E[h][w] = Src[w]; }
    Src += SrcStride;
  }
}
template<uint32 R, uint32 C, typename T> void xMatrix<R, C, T>::store(T* Dst, uint32 DstRows, uint32 DstCols, uint32 DstStride) const
{
  const uint32 NumRows = xMin(R, DstRows);
  const uint32 NumCols = xMin(C, DstCols);
  for(uint32 h = 0; h < NumRows; h++)
  { 
    for(uint32 w = 0; w < NumCols; w++) { Dst[w] = m_E[h][w]; }
    Dst += DstStride;
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// type convertion
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <uint32 R, uint32 C, typename T> template <typename OtherType> void xMatrix<R, C, T>::setConverted(const xMatrix<R, C, OtherType>& Src)
{
  if constexpr((std::is_same_v<T, float> && std::is_same_v<OtherType, double>) || (std::is_same_v<T, double> && std::is_same_v<OtherType, float>))
  {
    if constexpr(R == 4 && C == 4) { xMatrixUtils4x4::MatrixConvert4x4(m_E[0].data(), Src[0].data()); }
    else                           { xMatrixUtilsNxM<R, C>::Convert(m_E[0].data(), Src[0].data()); }
  }
  else
  {
    for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { m_E[h][w] = (T)(Src[h][w]); } }
  }
}
template <uint32 R, uint32 C, typename T> template <typename OtherType> xMatrix<R, C, OtherType> xMatrix<R, C, T>::getConverted() const
{
  xMatrix<R, C, OtherType> D;

  if constexpr((std::is_same_v<T, float> && std::is_same_v<OtherType, double>) || (std::is_same_v<T, double> && std::is_same_v<OtherType, float>))
  {
    if constexpr(R == 4 && C == 4) { xMatrixUtils4x4::MatrixConvert4x4(D[0].data(), m_E[0].data()); }
    else                           { xMatrixUtilsNxM<R, C>::Convert(D[0].data(), m_E[0].data()); }
  }
  else
  {
    for(uint32 h = 0; h < R; h++) { for(uint32 w = 0; w < C; w++) { D[h][w] = (T)(m_E[h][w]); } }
  }
  return D;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// matrix operations - compare
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> bool xMatrix<R, C, T>::isEqual(const xMatrix<R, C, T>& M) const
{
  for(uint32 h = 0; h < R; h++)
  {
    for(uint32 w = 0; w < C; w++)
    {
      if(m_E[h][w] != M[h][w]) { return false; }
    }
  }
  return true;
}
template<uint32 R, uint32 C, typename T> bool xMatrix<R, C, T>::isApproximatelyEqual(const xMatrix<R, C, T>& M, T Tolerance) const
{
  for(uint32 h = 0; h < R; h++)
  {
    for(uint32 w = 0; w < C; w++)
    {
      if(!xIsApproximatelyEqual(m_E[h][w], M[h][w], Tolerance)) { return false; }
    }
  }
  return true;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// matrix operations - round close values
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <uint32 R, uint32 C, typename T> void xMatrix<R, C, T>::setRoundValuesCloseToZeroOrOne(const xMatrix<R, C, T>& M, T Tolerance)
{ 
  for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { m_E[i][j] = xRoundValuesCloseToZeroOrOne(M[i][j], Tolerance); } }
}
template <uint32 R, uint32 C, typename T>  xMatrix<R, C, T>  xMatrix<R, C, T>::getRoundValuesCloseToZeroOrOne(T Tolerance) const
{ 
  xMatrix<R, C, T> D;
  for(uint32 i = 0; i < R; i++) { for(uint32 j = 0; j < C; j++) { D[i][j] = xRoundValuesCloseToZeroOrOne(m_E[i][j], Tolerance); } }
  return D; 
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// matrix operations - identity - only NxN matrix
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> bool xMatrix<R, C, T>::isIdentity() const
{
  if constexpr(R != C) { return false; }
  else
  {
    for(uint32 h = 0; h < R; h++)
    {
      for(uint32 w = 0; w < C; w++)
      {
        if(w == h) { if(!xIsApproximatelyOne (m_E[h][w])) { return false; } } //main diagonal
        else       { if(!xIsApproximatelyZero(m_E[h][w])) { return false; } }
      }
    } 
    return true;
  }
}
template<uint32 R, uint32 C, typename T> bool xMatrix<R, C, T>::isCloseToIdentity(T ToleranceOffset) const
{
  if constexpr(R != C) { return false; }
  else
  {
    for(uint32 h = 0; h < R; h++)
    {
      for(uint32 w = 0; w < C; w++)
      {
        if(w == h) { if(!xIsApproximatelyOne (m_E[h][w], ToleranceOffset)) { return false; } } //main diagonal
        else       { if(!xIsApproximatelyZero(m_E[h][w], ToleranceOffset)) { return false; } }
      }
    }
    return true;
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// matrix operations - inversion - only NxN matrix
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, void>::type xMatrix<R, C, T>::setInvertion(const xMatrix<R, C, T>& Src)
{
  if constexpr (R == 1)
  {
    m_E[0][0] = (T)1.0 / Src[0][0];
  }
  else if constexpr (R == 2)
  {
    T Det    = (T)1.0 / (Src[0][0] * Src[1][1] - Src[0][1] * Src[1][0]);
    T InvDet = (T)1.0 / Det;
    m_E[0][0] =  Src[1][1] * InvDet;
    m_E[0][1] = -Src[0][1] * InvDet;
    m_E[1][0] = -Src[1][0] * InvDet;
    m_E[1][1] =  Src[0][0] * InvDet;
  }
  else if constexpr (R == 3)
  {
    T M[9];
    for (uint32 h = 0, pp = 0; h < 3; h++) { for (uint32 w = 0; w < 3; w++, pp++) { M[pp] = Src[h][w]; } }

    T Inv[9];
    Inv[0] = M[4] * M[8] - M[7] * M[5];
    Inv[1] = M[2] * M[7] - M[1] * M[8];
    Inv[2] = M[1] * M[5] - M[2] * M[4];
    Inv[3] = M[5] * M[6] - M[3] * M[8];
    Inv[4] = M[0] * M[8] - M[2] * M[6];
    Inv[5] = M[3] * M[2] - M[0] * M[5];
    Inv[6] = M[3] * M[7] - M[6] * M[4];
    Inv[7] = M[6] * M[1] - M[0] * M[7];
    Inv[8] = M[0] * M[4] - M[3] * M[1];

    T Det    = M[0] * (Inv[0]) - M[1] * (M[3] * M[8] - M[5] * M[6]) + M[2] * (M[3] * M[7] - M[4] * M[6]);
    T InvDet = (T)1.0 / Det;

    for (int h = 0, pp = 0; h < 3; h++) { for (int w = 0; w < 3; w++, pp++) { m_E[h][w] = Inv[pp] * InvDet; } }
  }
  else if constexpr (R == 4)
  {
    T M[16];
    for(uint32 h = 0, pp = 0; h < 4; h++) { for(uint32 w = 0; w < 4; w++, pp++) { M[pp] = Src.at(h, w); } }

    T Inv[16];
    Inv[ 0] =  M[ 5] * M[10] * M[15] - M[ 5] * M[11] * M[14] - M[ 9] * M[ 6] * M[15] + M[ 9] * M[ 7] * M[14] + M[13] * M[ 6] * M[11] - M[13] * M[ 7] * M[10];
    Inv[ 4] = -M[ 4] * M[10] * M[15] + M[ 4] * M[11] * M[14] + M[ 8] * M[ 6] * M[15] - M[ 8] * M[ 7] * M[14] - M[12] * M[ 6] * M[11] + M[12] * M[ 7] * M[10];
    Inv[ 8] =  M[ 4] * M[ 9] * M[15] - M[ 4] * M[11] * M[13] - M[ 8] * M[ 5] * M[15] + M[ 8] * M[ 7] * M[13] + M[12] * M[ 5] * M[11] - M[12] * M[ 7] * M[ 9];
    Inv[12] = -M[ 4] * M[ 9] * M[14] + M[ 4] * M[10] * M[13] + M[ 8] * M[ 5] * M[14] - M[ 8] * M[ 6] * M[13] - M[12] * M[ 5] * M[10] + M[12] * M[ 6] * M[ 9];
    Inv[ 1] = -M[ 1] * M[10] * M[15] + M[ 1] * M[11] * M[14] + M[ 9] * M[ 2] * M[15] - M[ 9] * M[ 3] * M[14] - M[13] * M[ 2] * M[11] + M[13] * M[ 3] * M[10];
    Inv[ 5] =  M[ 0] * M[10] * M[15] - M[ 0] * M[11] * M[14] - M[ 8] * M[ 2] * M[15] + M[ 8] * M[ 3] * M[14] + M[12] * M[ 2] * M[11] - M[12] * M[ 3] * M[10];
    Inv[ 9] = -M[ 0] * M[ 9] * M[15] + M[ 0] * M[11] * M[13] + M[ 8] * M[ 1] * M[15] - M[ 8] * M[ 3] * M[13] - M[12] * M[ 1] * M[11] + M[12] * M[ 3] * M[ 9];
    Inv[13] =  M[ 0] * M[ 9] * M[14] - M[ 0] * M[10] * M[13] - M[ 8] * M[ 1] * M[14] + M[ 8] * M[ 2] * M[13] + M[12] * M[ 1] * M[10] - M[12] * M[ 2] * M[ 9];
    Inv[ 2] =  M[ 1] * M[ 6] * M[15] - M[ 1] * M[ 7] * M[14] - M[ 5] * M[ 2] * M[15] + M[ 5] * M[ 3] * M[14] + M[13] * M[ 2] * M[ 7] - M[13] * M[ 3] * M[ 6];
    Inv[ 6] = -M[ 0] * M[ 6] * M[15] + M[ 0] * M[ 7] * M[14] + M[ 4] * M[ 2] * M[15] - M[ 4] * M[ 3] * M[14] - M[12] * M[ 2] * M[ 7] + M[12] * M[ 3] * M[ 6];
    Inv[10] =  M[ 0] * M[ 5] * M[15] - M[ 0] * M[ 7] * M[13] - M[ 4] * M[ 1] * M[15] + M[ 4] * M[ 3] * M[13] + M[12] * M[ 1] * M[ 7] - M[12] * M[ 3] * M[ 5];
    Inv[14] = -M[ 0] * M[ 5] * M[14] + M[ 0] * M[ 6] * M[13] + M[ 4] * M[ 1] * M[14] - M[ 4] * M[ 2] * M[13] - M[12] * M[ 1] * M[ 6] + M[12] * M[ 2] * M[ 5];
    Inv[ 3] = -M[ 1] * M[ 6] * M[11] + M[ 1] * M[ 7] * M[10] + M[ 5] * M[ 2] * M[11] - M[ 5] * M[ 3] * M[10] - M[ 9] * M[ 2] * M[ 7] + M[ 9] * M[ 3] * M[ 6];
    Inv[ 7] =  M[ 0] * M[ 6] * M[11] - M[ 0] * M[ 7] * M[10] - M[ 4] * M[ 2] * M[11] + M[ 4] * M[ 3] * M[10] + M[ 8] * M[ 2] * M[ 7] - M[ 8] * M[ 3] * M[ 6];
    Inv[11] = -M[ 0] * M[ 5] * M[11] + M[ 0] * M[ 7] * M[ 9] + M[ 4] * M[ 1] * M[11] - M[ 4] * M[ 3] * M[ 9] - M[ 8] * M[ 1] * M[ 7] + M[ 8] * M[ 3] * M[ 5];
    Inv[15] =  M[ 0] * M[ 5] * M[10] - M[ 0] * M[ 6] * M[ 9] - M[ 4] * M[ 1] * M[10] + M[ 4] * M[ 2] * M[ 9] + M[ 8] * M[ 1] * M[ 6] - M[ 8] * M[ 2] * M[ 5];
       
    T Det    = M[0] * Inv[0] + M[1] * Inv[4] + M[2] * Inv[8] + M[3] * Inv[12];
    T InvDet = (T)1.0 / Det;

    for(int h = 0, pp = 0; h < 4; h++) { for(int w = 0; w < 4; w++, pp++) { m_E[h][w] = Inv[pp] * InvDet; } }
  }
  else
  {
    abort();
  }
  assert(R<=4);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// multiplication
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> template<uint32 S> void xMatrix<R, C, T>::setMultiplyMatrixByMatrix(const xMatrix<R, S, T>& A, const xMatrix<S, C, T>& B)
{
  //this(Rows x Cols) = A(Rows x Size) * B(Size x Cols)
  if constexpr (R == 4 && C == 4 && S == 4 && std::is_same_v<T, float>)
  { 
    xMatrixUtils4x4::MatrixMultiply4x4(m_E[0].data(), A[0].data(), B[0].data());
  }
  else
  {
    for(uint32 i = 0; i < R; i++)
    {
      for(uint32 j = 0; j < C; j++)
      {
        T Tmp = (T)0;
        for(uint32 k = 0; k < S; k++) { Tmp += (A[i][k] * B[k][j]); }
        m_E[i][j] = Tmp;
      }
    }
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// print
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> std::string xMatrix<R, C, T>::print(const std::string& Prefix1st, const std::string& PrefixNth, const std::string& Separator) const
{
  std::string Result;
  for(uint32 h = 0; h < R; h++)
  { 
    Result += ((h == 0) ? Prefix1st : PrefixNth);
    for(uint32 w = 0; w < C; w++)
    {
      Result += fmt::sprintf("%14.8f", m_E[h][w]);
      if(h < R - 1 || w < C - 1) { Result += Separator; }
    } 
    Result += "\n";
  }
  return Result;
}
template<uint32 R, uint32 C, typename T> std::string xMatrix<R, C, T>::print(const std::string& Prefix1st, const std::string& PrefixNth, const std::string& Separator, uint32 NumRows, uint32 NumCols) const
{
  NumRows = xMin(R, NumRows);
  NumCols = xMin(C, NumCols);

  std::string Result;
  for(uint32 h = 0; h < NumRows; h++)
  { 
    Result += ((h == 0) ? Prefix1st : PrefixNth);
    for(uint32 w = 0; w < NumCols; w++)
    {
      Result += fmt::sprintf("%14.8f", m_E[h][w]);
      if(h < NumRows - 1 || w < NumCols - 1) { Result += Separator; }
    } 
    Result += "\n";
  }
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// experimental - Lower / Upper inversion
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::InvertLowerTriangular(xMatrix<R, C, T>& InvL, const xMatrix<R, C, T>& L)
{
  static constexpr uint32 n = C;
  InvL.zero();

  // Solve L * x = e_i for each column i of invL
  for(uint32 i = 0; i < n; i++)
  {
    for(uint32 j = 0; j < n; j++)
    {
      T Rhs = (i == j) ? 1.0 : 0.0; // e_i[j]
      T Sum = 0.0;
      for(uint32 k = 0; k < j; k++)
      {
        Sum += L[j][k] * InvL[k][i];
      }
      if(xIsApproximatelyZero(L[j][j])) { fmt::print("Error: Zero diagonal element at L[{}][{}]\n", j, j); return false; }
      InvL[j][i] = (Rhs - Sum) / L[j][j];
    }
  }
  return true;


  // Solve L x = e_j for each column j of L_inv using forward substitution
  //for(int j = 0; j < n; j++) {
  //  for(int i = 0; i < n; i++) {
  //    if(fabs(L[i][i]) < 1e-10) {
  //      printf("Error: Singular matrix, zero diagonal at L[%d][%d]\n", i, i);
  //      exit(1);
  //    }
  //    // Compute L_inv[i][j] = (b_i - sum_{k=0}^{i-1} L[i][k] * L_inv[k][j]) / L[i][i]
  //    // where b_i = 1 if i == j, else 0
  //    double sum = 0.0;
  //    for(int k = 0; k < i; k++)
  //      sum += L[i][k] * L_inv[k][j];
  //    L_inv[i][j] = ((i == j ? 1.0 : 0.0) - sum) / L[i][i];
  //  }
  //}
}
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::InvertUpperTriangular(xMatrix<R, C, T>& InvU, const xMatrix<R, C, T>& U)
{
  static constexpr int32 n = C;
  InvU.zero();

  // Solve U x = e_j for each column j of InvU using back substitution
  for(int32 j = 0; j < n; j++)
  {
    for(int32 i = n - 1; i >= 0; i--)
    {
      // Check for singular matrix
      if(xIsApproximatelyZero(U[i][i])) { fmt::print("Error: Singular matrix, zero diagonal at U[{}][{}]\n", i, i); return false; }
      // Compute InvU[i][j] = (b_i - sum_{k=i+1}^{n-1} U[i][k] * InvU[k][j]) / U[i][i]
      // where b_i = 1 if i == j, else 0
      T Sum = 0.0;
      for(int32 k = i + 1; k < n; k++)
      {
        Sum += U[i][k] * InvU[k][j];
      }
      InvU[i][j] = ((i == j ? 1.0 : 0.0) - Sum) / U[i][i];
    }
  }

  //// Solve U x = e_j for each column j of U_inv using back substitution
  //for(int j = 0; j < n; j++) {
  //  for(int i = n - 1; i >= 0; i--) {
  //    if(fabs(U[i][i]) < 1e-10) {
  //      printf("Error: Singular matrix, zero diagonal at U[%d][%d]\n", i, i);
  //      exit(1);
  //    }
  //    // Compute U_inv[i][j] = (b_i - sum_{k=i+1}^{n-1} U[i][k] * U_inv[k][j]) / U[i][i]
  //    // where b_i = 1 if i == j, else 0
  //    double sum = 0.0;
  //    for(int k = i + 1; k < n; k++)
  //      sum += U[i][k] * U_inv[k][j];
  //    U_inv[i][j] = ((i == j ? 1.0 : 0.0) - sum) / U[i][i];
  //  }
  //}

  return true;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// experimental - Cholesky inversion - only for symmetric positive definite metrices
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::DecomposeCholesky(xMatrix<R, C, T>& L, const xMatrix<R, C, T>& A)
{
  static constexpr uint32 n = C;
  L.zero();

  for(uint32 i = 0; i < n; i++)
  {
    for(uint32 j = 0; j <= i; j++)
    {
      T Sum = 0;
      if(i == j)  // Diagonal elements
      {
        for(uint32 k = 0; k < j; k++)
        {
          Sum += L[j][k] * L[j][k];
        }
        if(A[j][j] - Sum <= 0) { fmt::print("Error: Matrix is not positive definite at index {}\n", j); return false; }
        L[j][j] = sqrt(A[j][j] - Sum);
      }
      else // Off-diagonal elements
      {
        for(uint32 k = 0; k < j; k++)
        {
          Sum += L[i][k] * L[j][k];
        }
        if(L[j][j] == 0) { fmt::print("Error: Zero diagonal element in L at index {}\n", j); return false; }
        L[i][j] = (A[i][j] - Sum) / L[j][j];
      }
    }
  }
  return true;
}
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::InvertCholesky(xMatrix<R, C, T>& InvA, const xMatrix<R, C, T>& A)
{
  xMatrix<R, C, T> L, InvL;
  bool ResDC = xMatrix<R, C, T>::DecomposeCholesky(L, A);
  if(!ResDC) { return false; }
  bool ResIL = xMatrix<R, C, T>::InvertLowerTriangular(InvL, L);
  if(!ResIL) { return false; }
  static constexpr uint32 n = C;
  // InvA = (invL)^T * invL
  for(uint32 i = 0; i < n; i++)
  {
    for(uint32 j = 0; j < n; j++)
    {
      InvA[i][j] = 0.0;
      for(uint32 k = 0; k < n; k++)
      {
        InvA[i][j] += InvL[k][i] * InvL[k][j]; // (InvL)^T[i][k] = InvL[k][i]
      }
    }
  }
  return true;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// experimental - LU inversion
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::DecomposeLU(xMatrix<R, C, T>& L, xMatrix<R, C, T>& U, const xMatrix<R, C, T>& A)
{
  static constexpr uint32 n = C;

  L.setIdentity();
  U.zero       ();

  // LU decomposition (Doolittle's method)
  for(int32 k = 0; k < n; k++)
  {
    // Compute U[k][k] to U[k][n-1]
    U[k][k] = A[k][k];
    for(int32 i = 0; i < k; i++)
    {
      U[k][k] -= L[k][i] * U[i][k];
    }
    if(xIsApproximatelyZero(U[k][k])) { fmt::print("Error: Zero pivot at U[{}][{}]\n", k, k); return false; }

    // Compute U[k][j] for j > k
    for(int32 j = k + 1; j < n; j++)
    {
      U[k][j] = A[k][j];
      for(int32 i = 0; i < k; i++)
      {
        U[k][j] -= L[k][i] * U[i][j];
      }
    }

    // Compute L[j][k] for j > k
    for(int32 j = k + 1; j < n; j++)
    {
      L[j][k] = A[j][k];
      for(int32 i = 0; i < k; i++)
      {
        L[j][k] -= L[j][i] * U[i][k];
      }
      L[j][k] /= U[k][k];
    }
  }

  return true;
}
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::InvertLU(xMatrix<R, C, T>& InvA, const xMatrix<R, C, T>& A)
{
  static constexpr uint32 n = C;

  xMatrix<R, C, T> L;
  xMatrix<R, C, T> U;

  bool ResLU = DecomposeLU(L, U, A); if(!ResLU) { return false; }  

  xMatrix<R, C, T> InvL;
  xMatrix<R, C, T> InvU;

  bool ResIL = InvertLowerTriangular(InvL, L); if(!ResIL) { return false; }
  bool ResIU = InvertUpperTriangular(InvU, U); if(!ResIU) { return false; }

  InvA = InvU.getMultiplyByMatrix(InvL);

  return true;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// experimental - QR inversion
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::DecomposeQR(xMatrix<R, C, T>& Q, xMatrix<R, C, T>& U, const xMatrix<R, C, T>& A)
{
  static constexpr uint32 n = C;

  auto norm = [](T* v, int32 n) -> T 
    { 
      T Sum = 0.0;
      for(int32 i = 0; i < n; i++) { Sum += v[i] * v[i]; }
      return sqrt(Sum);
    };

  Q.setIdentity();
  U = A;

  std::array<T, n> x;
  std::array<T, n> e;

  // Householder transformations
  for(uint32 k = 0; k < n - 1; k++)
  {
    // Extract column k below diagonal
    for(uint32 i = k; i < n; i++)
    {
      x[i - k] = U[i][k];
    }

    // Compute Householder vector
    T alpha = norm(x.data(), n - k);
    if(alpha == 0) { fmt::print("Error: Matrix is singular or nearly singular at column {}\n", k); return false; }

    for(uint32 i = 0; i < n - k; i++)
    {
      e[i] = (i == 0) ? 1.0 : 0.0;
    }
    T sign = (x[0] >= 0) ? 1.0 : -1.0;
    for(uint32 i = 0; i < n - k; i++)
    {
      x[i] = x[i] + sign * alpha * e[i];
    }
    T beta = norm(x.data(), n - k);
    if(beta == 0) { continue; }
    for(uint32 i = 0; i < n - k; i++)
    {
      x[i] /= beta;
    }

    // Apply Householder reflection to R
    for(int32 j = k; j < n; j++)
    {
      T Sum = 0.0;
      for(uint32 i = k; i < n; i++)
      {
        Sum += x[i - k] * U[i][j];
      }
      for(uint32 i = k; i < n; i++)
      {
        U[i][j] -= 2.0 * Sum * x[i - k];
      }
    }

    // Apply Householder reflection to Q
    for(uint32 j = 0; j < n; j++)
    {
      T Sum = 0.0;
      for(uint32 i = k; i < n; i++)
      {
        Sum += x[i - k] * Q[j][i];
      }
      for(uint32 i = k; i < n; i++)
      {
        Q[j][i] -= 2.0 * Sum * x[i - k];
      }
    }
  }

  return true;
}
template<uint32 R, uint32 C, typename T> template <uint32 RR, uint32 CC> typename std::enable_if<RR == CC, bool>::type xMatrix<R, C, T>::InvertQR(xMatrix<R, C, T>& InvA, const xMatrix<R, C, T>& A)
{
  xMatrix<R, C, T> Q;
  xMatrix<R, C, T> U;

  bool ResQR = DecomposeQR(Q, U, A); if(!ResQR) { return false; }

  xMatrix<R, C, T> InvU;

  bool ResIU = InvertUpperTriangular(InvU, U); if(!ResIU) { return false; }

  // InvA = InvU * Q^T
  static constexpr uint32 n = C;
  for(int32 i = 0; i < n; i++)
  {
    for(int32 j = 0; j < n; j++)
    {
      InvA[i][j] = 0.0;
      for(int32 k = 0; k < n; k++)
      {
        InvA[i][j] += InvU[i][k] * Q[j][k]; // Q^T[k][j] = Q[j][k]
      }
    }
  }

  return true;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// type alias
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using flt32M4x4 = xMatrix<4, 4, flt32>;
using flt32M3x4 = xMatrix<3, 4, flt32>;
using flt32M3x3 = xMatrix<3, 3, flt32>;
using flt32M3x1 = xMatrix<3, 1, flt32>;
using flt64M4x4 = xMatrix<4, 4, flt64>;
using flt64M3x4 = xMatrix<3, 4, flt64>;
using flt64M3x3 = xMatrix<3, 3, flt64>;
using flt64M3x1 = xMatrix<3, 1, flt64>;

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//instantiation of most often types
#ifndef PMBB_xMatrix_IMPLEMENTATION
extern template class xMatrix<4, 4, flt32>;
extern template class xMatrix<3, 4, flt32>;
extern template class xMatrix<3, 3, flt32>;
extern template class xMatrix<3, 1, flt32>;
extern template class xMatrix<4, 4, flt64>;
extern template class xMatrix<3, 4, flt64>;
extern template class xMatrix<3, 3, flt64>;
extern template class xMatrix<3, 1, flt64>;
#endif // !PMBB_xMatrix_IMPLEMENTATION

//===============================================================================================================================================================================================================

} //end of namespace PMBB

