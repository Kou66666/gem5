/*
 * Copyright (c) 2025
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ARCH_RISCV_REGS_MATRIX_HH__
#define __ARCH_RISCV_REGS_MATRIX_HH__

#include <array>
#include <string>
#include <vector>
#include "cpu/reg_class.hh"

namespace gem5
{

namespace RiscvISA
{

constexpr int MatrixRows = 64; // Default number of rows
constexpr int MatrixCols = 512; // Default number of bits in a column
constexpr int NumMatrixRegs = 8; // Number of matrix registers

using MatrixReg = std::array<std::array<uint64_t, MatrixCols / 64>, MatrixRows>;

const std::vector<std::string> MatrixRegNames = {
    "tmm0", "tmm1", "tmm2", "tmm3", "tmm4", "tmm5", "tmm6", "tmm7"
};

class MatrixRegFile
{
  public:
    MatrixRegFile() {
        for (auto &reg : regs) {
            for (auto &row : reg) {
                row.fill(0); // Initialize all elements to 0
            }
        }
    }

    MatrixReg &operator[](int idx) { return regs[idx]; }
    const MatrixReg &operator[](int idx) const { return regs[idx]; }

  private:
    std::array<MatrixReg, NumMatrixRegs> regs;
};

inline constexpr RegClass matrixRegClass =
    RegClass(RegClassType::VecRegClass, "MatrixRegs", NumMatrixRegs, debug::VecRegs);

} // namespace RiscvISA

} // namespace gem5

#endif // __ARCH_RISCV_REGS_MATRIX_HH__
