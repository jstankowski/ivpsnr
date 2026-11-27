#pragma once
#include "xCommonDefFVGS.h"
#include "xCommonDefCORE.h"

namespace PMBB_NAMESPACE::FVGS {

  //===============================================================================================================================================================================================================

  class xSphHarmEvaluator {
  public:
    static constexpr int L = 3;
    static constexpr int NumCoeffs = (L + 1) * (L + 1);

    xSphHarmEvaluator(const flt32* SH_R, const flt32* SH_G, const flt32* SH_B) 
    {
      std::copy(SH_R, SH_R + NumCoeffs, this->SH_R.begin());
      std::copy(SH_G, SH_G + NumCoeffs, this->SH_G.begin());
      std::copy(SH_B, SH_B + NumCoeffs, this->SH_B.begin());
    }

    flt32V3 xEvaluateAtDirection(flt32 x, flt32 y, flt32 z) const 
    {
      flt32 theta = std::acos(std::clamp(z, -1.0f, 1.0f));
      flt32 phi = std::atan2(y, x);
      flt32 r = 0, g = 0, b = 0;

      for (int l = 0; l <= L; ++l)
        for (int m = -l; m <= l; ++m) 
        {
          int i = l * l + l + m;
          flt32 Y = xSH_Ylm(l, m, theta, phi);
          r += SH_R[i] * Y;
          g += SH_G[i] * Y;
          b += SH_B[i] * Y;
        }
      return { r, g, b };
    }

    static flt32 xSH_Ylm(int l, int m, flt32 theta, flt32 phi) {
      flt32 x = std::cos(theta);
      if      (m > 0) { return std::sqrt(2.0f) * K(l,  m) * std::cos( m * phi) * P(l,  m, x); }
      else if (m < 0) { return std::sqrt(2.0f) * K(l, -m) * std::sin(-m * phi) * P(l, -m, x); }
      else            { return                   K(l,  0) *                      P(l,  0, x); }
    }

  private:
    std::array<flt32, NumCoeffs> SH_R, SH_G, SH_B;
    static flt32 K(int l, int m) 
    {
      flt32 num = (2.0f * l + 1) * std::tgamma(l - m + 1);
      flt32 denom = 4.0f * xc_Pi<flt32> * std::tgamma(l + m + 1);
      return std::sqrt(num / denom);
    }

    static flt32 P(int l, int m, flt32 x) 
    {
      if (m < 0 || m > l) { return 0.0f; }
      flt32 pmm = 1.0f;
      if (m > 0) 
      {
        flt32 somx2 = std::sqrt((1.0f - x) * (1.0f + x));
        flt32 fact = 1.0f;
        for (int i = 1; i <= m; ++i) 
        {
          pmm *= -fact * somx2;
          fact += 2.0f;
        }
      }

      if (l == m) { return pmm; }
      flt32 pmmp1 = x * (2 * m + 1) * pmm;
      if (l == m + 1) { return pmmp1; }
      flt32 pll = 0.0f;
      for (int ll = m + 2; ll <= l; ++ll) 
      {
        pll = ((2 * ll - 1) * x * pmmp1 - (ll + m - 1) * pmm) / (ll - m);
        pmm = pmmp1;
        pmmp1 = pll;
      }
      return pll;
    }
  };

  class xSphHarmUtils 
  {
  public:
    // 1. Sampling: generate uniform points on a sphere (Fibonacci)
    static std::vector<flt32V3> xGenerateUniformSphereSamples(int nSamples) 
    {
      std::vector<flt32V3> points(nSamples);
      flt32 offset = 2.0f / nSamples;
      flt32 increment = xc_Pi<flt32> * (3.0f - std::sqrt(5.0f));
      for (int i = 0; i < nSamples; ++i) 
      {
        flt32 y = ((i * offset) - 1.0f) + (offset / 2.0f);
        flt32 r = std::sqrt(1.0f - y * y);
        flt32 phi = i * increment;
        flt32 x = std::cos(phi) * r;
        flt32 z = std::sin(phi) * r;
        points[i] = { x, y, z };
      }
      return points;
    }

    // 2. Evaluate SH on arbitrary list of directions (sampling)
    static std::vector<flt32V3> xEvaluateSphHarmOnDirections(const xSphHarmEvaluator& shEval, const std::vector<flt32V3>& directions) 
    {
      std::vector<flt32V3> result;
      result.reserve(directions.size());
      for (const auto& dir : directions) { result.push_back(shEval.xEvaluateAtDirection(dir[0], dir[1], dir[2])); }
      return result;
    }

    // 3. SH fitting from samples (classic/naive, no interpolation)
    static void xFitSphHarmFromSamples( const std::vector<flt32V3>& directions, const std::vector<flt32V3>& colors, flt32* SH_R, flt32* SH_G, flt32* SH_B)
    {
      constexpr int NumCoeffs = xSphHarmEvaluator::NumCoeffs;
      std::fill(SH_R, SH_R + NumCoeffs, 0.0f);
      std::fill(SH_G, SH_G + NumCoeffs, 0.0f);
      std::fill(SH_B, SH_B + NumCoeffs, 0.0f);
      flt32 weights[NumCoeffs] = {};
      for (size_t i = 0; i < directions.size(); ++i)
      {
        const auto& d = directions[i];
        const auto& rgb = colors[i];
        flt32 theta = std::acos(std::clamp(d[2], -1.0f, 1.0f));
        flt32 phi = std::atan2(d[1], d[0]);
        for (int l = 0; l <= xSphHarmEvaluator::L; ++l) 
        {
          for (int m = -l; m <= l; ++m) 
          {
            int idx = l * l + l + m;
            flt32 Y = xSphHarmEvaluator::xSH_Ylm(l, m, theta, phi);
            SH_R[idx] += rgb[0] * Y;
            SH_G[idx] += rgb[1] * Y;
            SH_B[idx] += rgb[2] * Y;
            weights[idx] += Y * Y;
          }
        }
      }
      for (int i = 0; i < NumCoeffs; ++i) 
      {
        if (weights[i] > 1e-6f) 
        {
          SH_R[i] /= weights[i];
          SH_G[i] /= weights[i];
          SH_B[i] /= weights[i];
        }
      }
    }

    // 4. SH fitting from samples with kernel-weighted (gaussian) interpolation, for arbitrary target grid
    static void xFitSphHarmFromSamplesInterpolated(const std::vector<flt32V3>& directions, const std::vector<flt32V3>& colors, flt32* SH_R, flt32* SH_G, flt32* SH_B, const std::vector<flt32V3>& fitGrid, flt32 sigma = 0.3f)
    {
      constexpr int NumCoeffs = xSphHarmEvaluator::NumCoeffs;
      std::fill(SH_R, SH_R + NumCoeffs, 0.0f);
      std::fill(SH_G, SH_G + NumCoeffs, 0.0f);
      std::fill(SH_B, SH_B + NumCoeffs, 0.0f);
      for (const auto& d : fitGrid) 
      {
        flt32 x = d[0], y = d[1], z = d[2];
        flt32 theta = std::acos(std::clamp(z, -1.0f, 1.0f));
        flt32 phi = std::atan2(y, x);
        auto rgb = xInterpolateSampleColor(x, y, z, directions, colors, sigma);
        for (int l = 0; l <= xSphHarmEvaluator::L; ++l) 
        {
          for (int m = -l; m <= l; ++m) 
          {
            int idx = l * l + l + m;
            flt32 Y = xSphHarmEvaluator::xSH_Ylm(l, m, theta, phi);
            SH_R[idx] += rgb[0] * Y;
            SH_G[idx] += rgb[1] * Y;
            SH_B[idx] += rgb[2] * Y;
          }
        }
      }
      int N = (int)fitGrid.size();
      if (N > 0) 
      {
        for (int i = 0; i < NumCoeffs; ++i) 
        {
          SH_R[i] /= N;
          SH_G[i] /= N;
          SH_B[i] /= N;
        }
      }
    }

    // 5. Generate equirectangular visualization from SH
    static std::vector<flt32V3> xGenerateEquirectangularMap(const xSphHarmEvaluator& shEval, int width, int height)
    {
      std::vector<flt32V3> image(width * height);
      for (int y = 0; y < height; ++y) 
      {
        flt32 v = flt32(y) / (height - 1);
        flt32 theta = v * xc_Pi<flt32>;
        for (int x = 0; x < width; ++x) 
        {
          flt32 u = flt32(x) / (width - 1);
          flt32 phi = u * 2.0f * xc_Pi<flt32>;
          flt32 dirX = std::sin(theta) * std::cos(phi);
          flt32 dirY = std::sin(theta) * std::sin(phi);
          flt32 dirZ = std::cos(theta);
          auto rgb = shEval.xEvaluateAtDirection(dirX, dirY, dirZ);
          image[y * width + x] = rgb;
        }
      }
      return image;
    }

    // 6. Fit SH coefficients directly from equirectangular image
    static void xFitSphHarmFromImage(const std::vector<flt32V3>& image, int width, int height, flt32* SH_R, flt32* SH_G, flt32* SH_B)
    {
      constexpr int NumCoeffs = xSphHarmEvaluator::NumCoeffs;
      std::fill(SH_R, SH_R + NumCoeffs, 0.0f);
      std::fill(SH_G, SH_G + NumCoeffs, 0.0f);
      std::fill(SH_B, SH_B + NumCoeffs, 0.0f);
      for (int y = 0; y < height; ++y) 
      {
        flt32 v = flt32(y) / (height - 1);
        flt32 theta = v * xc_Pi<flt32>;
        flt32 sinTheta = std::sin(theta);
        flt32 dOmega = (2.0f * xc_Pi<flt32> / width) * (xc_Pi<flt32> / height) * sinTheta;
        for (int x = 0; x < width; ++x) 
        {
          flt32 u = flt32(x) / (width - 1);
          flt32 phi = u * 2.0f * xc_Pi<flt32>;
          const auto& pixel = image[y * width + x];
          flt32 r = pixel[0], g = pixel[1], b = pixel[2];
          for (int l = 0; l <= xSphHarmEvaluator::L; ++l) 
          {
            for (int m = -l; m <= l; ++m) 
            {
              int idx = l * l + l + m;
              flt32 Y = xSphHarmEvaluator::xSH_Ylm(l, m, theta, phi);
              SH_R[idx] += r * Y * dOmega;
              SH_G[idx] += g * Y * dOmega;
              SH_B[idx] += b * Y * dOmega;
            }
          }
        }
      }
    }

    // 7. Overlay sampling points on equirectangular image (flt32 RGB)
    static void xOverlaySamplingPointsOnEquirectangular(std::vector<flt32V3>& image, int width, int height, const std::vector<flt32V3>& directions, const flt32V3& pointColor = { 1.0f, 1.0f, 1.0f }, int pointRadius = 2)
    {
      for (const auto& dir : directions) 
      {
        flt32 x = dir[0], y = dir[1], z = dir[2];
        flt32 theta = std::acos(std::clamp(z, -1.0f, 1.0f)); // 0..PI
        flt32 phi = std::atan2(y, x);                        // -PI..PI
        if (phi < 0) { phi += 2.0f * xc_Pi<flt32>; }         // 0..2PI

        int px = int((phi / (2.0f * xc_Pi<flt32>)) * (width - 1));
        int py = int((theta / xc_Pi<flt32>) * (height - 1));

        for (int dy = -pointRadius; dy <= pointRadius; ++dy) 
        {
          for (int dx = -pointRadius; dx <= pointRadius; ++dx) 
          {
            int cx = px + dx;
            int cy = py + dy;
            if (cx >= 0 && cx < width && cy >= 0 && cy < height) 
            {
              flt32 dist2 = dx * dx + dy * dy;
              if (dist2 <= pointRadius * pointRadius) { image[cy * width + cx] = pointColor; }
            }
          }
        }
      }
    }

    // --- Helpers ---
    static flt32 xGaussianKernel(flt32 cos_theta, flt32 sigma = 0.3f) 
    {
      flt32 theta = std::acos(std::clamp(cos_theta, -1.0f, 1.0f));
      return std::exp(-(theta * theta) / (2.0f * sigma * sigma));
    }

    static flt32V3 xInterpolateSampleColor(flt32 x, flt32 y, flt32 z, const std::vector<flt32V3>& directions, const std::vector<flt32V3>& colors, flt32 sigma = 0.3f)
    {
      flt32 sumR = 0, sumG = 0, sumB = 0, norm = 0;
      for (size_t i = 0; i < directions.size(); ++i) 
      {
        const auto& d = directions[i];
        flt32 dotp = x * d[0] + y * d[1] + z * d[2];
        dotp = std::clamp(dotp, -1.0f, 1.0f);
        flt32 w = xGaussianKernel(dotp, sigma);
        sumR += colors[i][0] * w;
        sumG += colors[i][1] * w;
        sumB += colors[i][2] * w;
        norm += w;
      }
      if (norm > 1e-6f) { return { sumR / norm, sumG / norm, sumB / norm }; }
      else              { return { 0, 0, 0 }; }
    }
  };

  /*
  void saveBMP(const std::vector<std::array<unsigned char, 3>>& img, int width, int height, const std::string& filename) 
  {
    std::ofstream f(filename, std::ios::out | std::ios::binary);
    if (!f) return;

    int rowSize = (3 * width + 3) & (~3);  // row size must be multiple of 4 bytes
    int dataSize = rowSize * height;
    int fileSize = 54 + dataSize;

    // BMP Header
    unsigned char bmpFileHeader[14] = {
        'B', 'M',
        (unsigned char)(fileSize), (unsigned char)(fileSize >> 8),
        (unsigned char)(fileSize >> 16), (unsigned char)(fileSize >> 24),
        0, 0, 0, 0, 54, 0, 0, 0
    };

    // DIB Header (BITMAPINFOHEADER)
    unsigned char bmpInfoHeader[40] = {
        40, 0, 0, 0,
        (unsigned char)(width), (unsigned char)(width >> 8),
        (unsigned char)(width >> 16), (unsigned char)(width >> 24),
        (unsigned char)(height), (unsigned char)(height >> 8),
        (unsigned char)(height >> 16), (unsigned char)(height >> 24),
        1, 0, 24, 0
    };

    f.write((char*)bmpFileHeader, 14);
    f.write((char*)bmpInfoHeader, 40);

    std::vector<unsigned char> row(rowSize);

    // BMP stores pixels bottom-to-top
    for (int y = height - 1; y >= 0; --y) 
    {
      for (int x = 0; x < width; ++x) 
      {
        const auto& px = img[y * width + x];
        row[x * 3 + 0] = px[2]; // B
        row[x * 3 + 1] = px[1]; // G
        row[x * 3 + 2] = px[0]; // R
      }
      f.write((char*)row.data(), rowSize);
    }

    f.close();
  }


  void saveBMPFromFloat(const std::string& filename, int width, int height, const std::vector<flt32V3>& data) 
  {
    std::vector<std::array<unsigned char, 3>> image(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
      image[i][0] = static_cast<unsigned char>(std::clamp(data[i][0], 0.f, 1.f) * 255);
      image[i][1] = static_cast<unsigned char>(std::clamp(data[i][1], 0.f, 1.f) * 255);
      image[i][2] = static_cast<unsigned char>(std::clamp(data[i][2], 0.f, 1.f) * 255);
    }
    saveBMP(image, width, height, filename);
  }
  //===============================================================================================================================================================================================================
  */
} //end of namespace PMBB



