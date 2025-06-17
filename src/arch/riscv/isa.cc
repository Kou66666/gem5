/*
 * Copyright (c) 2016 RISC-V Foundation
 * Copyright (c) 2016 The University of Virginia
 * Copyright (c) 2020 Barkhausen Institut
 * Copyright (c) 2022 Google LLC
 * Copyright (c) 2024 University of Rostock
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

#include "arch/riscv/isa.hh"

#include <ctime>
#include <set>
#include <sstream>

#include "arch/riscv/faults.hh"
#include "arch/riscv/insts/static_inst.hh"
#include "arch/riscv/interrupts.hh"
#include "arch/riscv/mmu.hh"
#include "arch/riscv/pagetable.hh"
#include "arch/riscv/pmp.hh"
#include "arch/riscv/pcstate.hh"
#include "arch/riscv/regs/float.hh"
#include "arch/riscv/regs/int.hh"
#include "arch/riscv/regs/mat.hh"
#include "arch/riscv/regs/misc.hh"
#include "arch/riscv/regs/vector.hh"
#include "base/bitfield.hh"
#include "base/compiler.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "cpu/base.hh"
#include "debug/Checkpoint.hh"
#include "debug/LLSC.hh"
#include "debug/MatRegs.hh"
#include "debug/RiscvMisc.hh"
#include "debug/VecRegs.hh"
#include "mem/packet.hh"
#include "mem/request.hh"
#include "params/RiscvISA.hh"
#include "sim/pseudo_inst.hh"

namespace gem5
{

namespace RiscvISA
{

const std::array<const char *, NUM_MISCREGS> MiscRegNames = [] {
    std::array<const char *, NUM_MISCREGS> names = {};
    names[MISCREG_PRV] = "PRV";
    names[MISCREG_ISA] = "ISA";
    names[MISCREG_VENDORID] = "VENDORID";
    names[MISCREG_ARCHID] = "ARCHID";
    names[MISCREG_IMPID] = "IMPID";
    names[MISCREG_HARTID] = "HARTID";
    names[MISCREG_STATUS] = "STATUS";
    names[MISCREG_IP] = "IP";
    names[MISCREG_IE] = "IE";
    names[MISCREG_CYCLE] = "CYCLE";
    names[MISCREG_TIME] = "TIME";
    names[MISCREG_INSTRET] = "INSTRET";
    names[MISCREG_HPMCOUNTER03] = "HPMCOUNTER03";
    names[MISCREG_HPMCOUNTER04] = "HPMCOUNTER04";
    names[MISCREG_HPMCOUNTER05] = "HPMCOUNTER05";
    names[MISCREG_HPMCOUNTER06] = "HPMCOUNTER06";
    names[MISCREG_HPMCOUNTER07] = "HPMCOUNTER07";
    names[MISCREG_HPMCOUNTER08] = "HPMCOUNTER08";
    names[MISCREG_HPMCOUNTER09] = "HPMCOUNTER09";
    names[MISCREG_HPMCOUNTER10] = "HPMCOUNTER10";
    names[MISCREG_HPMCOUNTER11] = "HPMCOUNTER11";
    names[MISCREG_HPMCOUNTER12] = "HPMCOUNTER12";
    names[MISCREG_HPMCOUNTER13] = "HPMCOUNTER13";
    names[MISCREG_HPMCOUNTER14] = "HPMCOUNTER14";
    names[MISCREG_HPMCOUNTER15] = "HPMCOUNTER15";
    names[MISCREG_HPMCOUNTER16] = "HPMCOUNTER16";
    names[MISCREG_HPMCOUNTER17] = "HPMCOUNTER17";
    names[MISCREG_HPMCOUNTER18] = "HPMCOUNTER18";
    names[MISCREG_HPMCOUNTER19] = "HPMCOUNTER19";
    names[MISCREG_HPMCOUNTER20] = "HPMCOUNTER20";
    names[MISCREG_HPMCOUNTER21] = "HPMCOUNTER21";
    names[MISCREG_HPMCOUNTER22] = "HPMCOUNTER22";
    names[MISCREG_HPMCOUNTER23] = "HPMCOUNTER23";
    names[MISCREG_HPMCOUNTER24] = "HPMCOUNTER24";
    names[MISCREG_HPMCOUNTER25] = "HPMCOUNTER25";
    names[MISCREG_HPMCOUNTER26] = "HPMCOUNTER26";
    names[MISCREG_HPMCOUNTER27] = "HPMCOUNTER27";
    names[MISCREG_HPMCOUNTER28] = "HPMCOUNTER28";
    names[MISCREG_HPMCOUNTER29] = "HPMCOUNTER29";
    names[MISCREG_HPMCOUNTER30] = "HPMCOUNTER30";
    names[MISCREG_HPMCOUNTER31] = "HPMCOUNTER31";
    names[MISCREG_HPMEVENT03] = "HPMEVENT03";
    names[MISCREG_HPMEVENT04] = "HPMEVENT04";
    names[MISCREG_HPMEVENT05] = "HPMEVENT05";
    names[MISCREG_HPMEVENT06] = "HPMEVENT06";
    names[MISCREG_HPMEVENT07] = "HPMEVENT07";
    names[MISCREG_HPMEVENT08] = "HPMEVENT08";
    names[MISCREG_HPMEVENT09] = "HPMEVENT09";
    names[MISCREG_HPMEVENT10] = "HPMEVENT10";
    names[MISCREG_HPMEVENT11] = "HPMEVENT11";
    names[MISCREG_HPMEVENT12] = "HPMEVENT12";
    names[MISCREG_HPMEVENT13] = "HPMEVENT13";
    names[MISCREG_HPMEVENT14] = "HPMEVENT14";
    names[MISCREG_HPMEVENT15] = "HPMEVENT15";
    names[MISCREG_HPMEVENT16] = "HPMEVENT16";
    names[MISCREG_HPMEVENT17] = "HPMEVENT17";
    names[MISCREG_HPMEVENT18] = "HPMEVENT18";
    names[MISCREG_HPMEVENT19] = "HPMEVENT19";
    names[MISCREG_HPMEVENT20] = "HPMEVENT20";
    names[MISCREG_HPMEVENT21] = "HPMEVENT21";
    names[MISCREG_HPMEVENT22] = "HPMEVENT22";
    names[MISCREG_HPMEVENT23] = "HPMEVENT23";
    names[MISCREG_HPMEVENT24] = "HPMEVENT24";
    names[MISCREG_HPMEVENT25] = "HPMEVENT25";
    names[MISCREG_HPMEVENT26] = "HPMEVENT26";
    names[MISCREG_HPMEVENT27] = "HPMEVENT27";
    names[MISCREG_HPMEVENT28] = "HPMEVENT28";
    names[MISCREG_HPMEVENT29] = "HPMEVENT29";
    names[MISCREG_HPMEVENT30] = "HPMEVENT30";
    names[MISCREG_HPMEVENT31] = "HPMEVENT31";
    names[MISCREG_TSELECT] = "TSELECT";
    names[MISCREG_TDATA1] = "TDATA1";
    names[MISCREG_TDATA2] = "TDATA2";
    names[MISCREG_TDATA3] = "TDATA3";
    names[MISCREG_DCSR] = "DCSR";
    names[MISCREG_DPC] = "DPC";
    names[MISCREG_DSCRATCH] = "DSCRATCH";
    names[MISCREG_MEDELEG] = "MEDELEG";
    names[MISCREG_MIDELEG] = "MIDELEG";
    names[MISCREG_MTVEC] = "MTVEC";
    names[MISCREG_MCOUNTEREN] = "MCOUNTEREN";
    names[MISCREG_MSCRATCH] = "MSCRATCH";
    names[MISCREG_MEPC] = "MEPC";
    names[MISCREG_MCAUSE] = "MCAUSE";
    names[MISCREG_MTVAL] = "MTVAL";
    names[MISCREG_PMPCFG0] = "PMPCFG0";
    names[MISCREG_PMPCFG1] = "PMPCFG1";
    names[MISCREG_PMPCFG2] = "PMPCFG2";
    names[MISCREG_PMPCFG3] = "PMPCFG3";
    names[MISCREG_PMPADDR00] = "PMPADDR00";
    names[MISCREG_PMPADDR01] = "PMPADDR01";
    names[MISCREG_PMPADDR02] = "PMPADDR02";
    names[MISCREG_PMPADDR03] = "PMPADDR03";
    names[MISCREG_PMPADDR04] = "PMPADDR04";
    names[MISCREG_PMPADDR05] = "PMPADDR05";
    names[MISCREG_PMPADDR06] = "PMPADDR06";
    names[MISCREG_PMPADDR07] = "PMPADDR07";
    names[MISCREG_PMPADDR08] = "PMPADDR08";
    names[MISCREG_PMPADDR09] = "PMPADDR09";
    names[MISCREG_PMPADDR10] = "PMPADDR10";
    names[MISCREG_PMPADDR11] = "PMPADDR11";
    names[MISCREG_PMPADDR12] = "PMPADDR12";
    names[MISCREG_PMPADDR13] = "PMPADDR13";
    names[MISCREG_PMPADDR14] = "PMPADDR14";
    names[MISCREG_PMPADDR15] = "PMPADDR15";
    names[MISCREG_SEDELEG] = "SEDELEG";
    names[MISCREG_SIDELEG] = "SIDELEG";
    names[MISCREG_STVEC] = "STVEC";
    names[MISCREG_SCOUNTEREN] = "SCOUNTEREN";
    names[MISCREG_SSCRATCH] = "SSCRATCH";
    names[MISCREG_SEPC] = "SEPC";
    names[MISCREG_SCAUSE] = "SCAUSE";
    names[MISCREG_STVAL] = "STVAL";
    names[MISCREG_SATP] = "SATP";
    names[MISCREG_SENVCFG] = "SENVCFG";
    names[MISCREG_UTVEC] = "UTVEC";
    names[MISCREG_USCRATCH] = "USCRATCH";
    names[MISCREG_UEPC] = "UEPC";
    names[MISCREG_UCAUSE] = "UCAUSE";
    names[MISCREG_UTVAL] = "UTVAL";
    names[MISCREG_FFLAGS] = "FFLAGS";
    names[MISCREG_FRM] = "FRM";
    names[MISCREG_VSTART] = "VSTART";
    names[MISCREG_VXSAT] = "VXSAT";
    names[MISCREG_VXRM] = "VXRM";
    names[MISCREG_VCSR] = "VCSR";
    names[MISCREG_VL] = "VL";
    names[MISCREG_VTYPE] = "VTYPE";
    names[MISCREG_VLENB] = "VLENB";
    names[MISCREG_NMIVEC] = "NMIVEC";
    names[MISCREG_NMIE] = "NMIE";
    names[MISCREG_NMIP] = "NMIP";
    names[MISCREG_MSTATUSH] = "MSTATUSH";
    names[MISCREG_CYCLEH] = "CYCLEH";
    names[MISCREG_TIMEH] = "TIMEH";
    names[MISCREG_INSTRETH] = "INSTRETH";
    names[MISCREG_HPMCOUNTER03H] = "HPMCOUNTER03H";
    names[MISCREG_HPMCOUNTER04H] = "HPMCOUNTER04H";
    names[MISCREG_HPMCOUNTER05H] = "HPMCOUNTER05H";
    names[MISCREG_HPMCOUNTER06H] = "HPMCOUNTER06H";
    names[MISCREG_HPMCOUNTER07H] = "HPMCOUNTER07H";
    names[MISCREG_HPMCOUNTER08H] = "HPMCOUNTER08H";
    names[MISCREG_HPMCOUNTER09H] = "HPMCOUNTER09H";
    names[MISCREG_HPMCOUNTER10H] = "HPMCOUNTER10H";
    names[MISCREG_HPMCOUNTER11H] = "HPMCOUNTER11H";
    names[MISCREG_HPMCOUNTER12H] = "HPMCOUNTER12H";
    names[MISCREG_HPMCOUNTER13H] = "HPMCOUNTER13H";
    names[MISCREG_HPMCOUNTER14H] = "HPMCOUNTER14H";
    names[MISCREG_HPMCOUNTER15H] = "HPMCOUNTER15H";
    names[MISCREG_HPMCOUNTER16H] = "HPMCOUNTER16H";
    names[MISCREG_HPMCOUNTER17H] = "HPMCOUNTER17H";
    names[MISCREG_HPMCOUNTER18H] = "HPMCOUNTER18H";
    names[MISCREG_HPMCOUNTER19H] = "HPMCOUNTER19H";
    names[MISCREG_HPMCOUNTER20H] = "HPMCOUNTER20H";
    names[MISCREG_HPMCOUNTER21H] = "HPMCOUNTER21H";
    names[MISCREG_HPMCOUNTER22H] = "HPMCOUNTER22H";
    names[MISCREG_HPMCOUNTER23H] = "HPMCOUNTER23H";
    names[MISCREG_HPMCOUNTER24H] = "HPMCOUNTER24H";
    names[MISCREG_HPMCOUNTER25H] = "HPMCOUNTER25H";
    names[MISCREG_HPMCOUNTER26H] = "HPMCOUNTER26H";
    names[MISCREG_HPMCOUNTER27H] = "HPMCOUNTER27H";
    names[MISCREG_HPMCOUNTER28H] = "HPMCOUNTER28H";
    names[MISCREG_HPMCOUNTER29H] = "HPMCOUNTER29H";
    names[MISCREG_HPMCOUNTER30H] = "HPMCOUNTER30H";
    names[MISCREG_HPMCOUNTER31H] = "HPMCOUNTER31H";
    names[MISCREG_FFLAGS_EXE] = "FFLAGS_EXE";
    names[MISCREG_TMCSR] = "TMCSR";
    names[MISCREG_TMDATA] = "TMDATA";
    names[MISCREG_TMSIZE] = "TMSIZE";
    names[MISCREG_TMM] = "TMM";
    names[MISCREG_TMN] = "TMN";
    return names;
}();

namespace
{

/* Not applicable to RISCV */
RegClass vecElemClass(VecElemClass, VecElemClassName, 0, debug::IntRegs);
RegClass vecPredRegClass(VecPredRegClass, VecPredRegClassName, 0,
        debug::IntRegs);
// RegClass matRegClass(MatRegClass, MatRegClassName, 0, debug::MatRegs);
RegClass ccRegClass(CCRegClass, CCRegClassName, 0, debug::IntRegs);

} // anonymous namespace

ISA::ISA(const Params &p) : BaseISA(p, "riscv"),
    _rvType(p.riscv_type), enableRvv(p.enable_rvv), vlen(p.vlen), elen(p.elen),
    _privilegeModeSet(p.privilege_mode_set),
    _wfiResumeOnPending(p.wfi_resume_on_pending), _enableZcd(p.enable_Zcd)
{
    _regClasses.push_back(&intRegClass);
    _regClasses.push_back(&floatRegClass);
    _regClasses.push_back(&vecRegClass);
    _regClasses.push_back(&vecElemClass);
    _regClasses.push_back(&vecPredRegClass);
    _regClasses.push_back(&matRegClass);
    _regClasses.push_back(&ccRegClass);
    _regClasses.push_back(&miscRegClass);

    fatal_if( p.vlen < p.elen,
    "VLEN should be greater or equal",
        "than ELEN. Ch. 2RISC-V vector spec.");

    inform("RVV enabled, VLEN = %d bits, ELEN = %d bits",
            p.vlen, p.elen);


    miscRegFile.resize(NUM_PHYS_MISCREGS);
    clear();
}

bool ISA::inUserMode() const
{
    return miscRegFile[MISCREG_PRV] == PRV_U;
}

void
ISA::copyRegsFrom(ThreadContext *src)
{
    // First loop through the integer registers.
    for (auto &id: intRegClass)
        tc->setReg(id, src->getReg(id));

    // Second loop through the float registers.
    for (auto &id: floatRegClass)
        tc->setReg(id, src->getReg(id));

    // Third loop through the vector registers.
    RiscvISA::VecRegContainer vc;
    for (auto &id: vecRegClass) {
        src->getReg(id, &vc);
        tc->setReg(id, &vc);
    }

    // Copying Misc Regs
    for (int i = 0; i < NUM_PHYS_MISCREGS; i++)
        tc->setMiscRegNoEffect(i, src->readMiscRegNoEffect(i));

    // Lastly copy PC/NPC
    tc->pcState(src->pcState());
}

void ISA::clear()
{
    std::fill(miscRegFile.begin(), miscRegFile.end(), 0);

    miscRegFile[MISCREG_PRV] = PRV_M;
    miscRegFile[MISCREG_VENDORID] = 0;
    miscRegFile[MISCREG_ARCHID] = 0;
    miscRegFile[MISCREG_IMPID] = 0;

    MISA misa = 0;
    STATUS status = 0;

    // default config arch isa string is rv64(32)imafdc
    misa.rvi = misa.rvm = misa.rva = misa.rvf = misa.rvd = misa.rvc = 1;

    switch (getPrivilegeModeSet()) {
        case enums::M:
          break;
        case enums::MU:
          misa.rvu = 1;
          break;
        case enums::MNU:
          misa.rvu = misa.rvn = 1;
          break;
        case enums::MSU:
          misa.rvs = misa.rvu = 1;
          break;
        case enums::MNSU:
          misa.rvs = misa.rvu = misa.rvn = 1;
          break;
        default:
          panic("Privilege mode set config should not reach here");
    }

    // mark FS is initial
    status.fs = INITIAL;

    // _rvType dependent init.
    switch (_rvType) {
        case RV32:
          misa.rv32_mxl = 1;
          break;
        case RV64:
          misa.rv64_mxl = 2;
          status.uxl = status.sxl = 2;
          if (getEnableRvv()) {
              status.vs = VPUStatus::INITIAL;
              misa.rvv = 1;
          }
          break;
        default:
          panic("%s: Unknown _rvType: %d", name(), (int)_rvType);
    }

    miscRegFile[MISCREG_ISA] = misa;
    miscRegFile[MISCREG_STATUS] = status;
    miscRegFile[MISCREG_MCOUNTEREN] = 0x7;
    miscRegFile[MISCREG_SCOUNTEREN] = 0x7;
    // don't set it to zero; software may try to determine the supported
    // triggers, starting at zero. simply set a different value here.
    miscRegFile[MISCREG_TSELECT] = 1;
    // NMI is always enabled.
    miscRegFile[MISCREG_NMIE] = 1;
}

bool
ISA::hpmCounterEnabled(int misc_reg) const
{
    int hpmcounter = 0;
    if (misc_reg >= MISCREG_CYCLEH) {
        hpmcounter = misc_reg - MISCREG_CYCLEH;
    } else {
        hpmcounter = misc_reg - MISCREG_CYCLE;
    }

    if (hpmcounter < 0 || hpmcounter > 31)
        panic("Illegal HPM counter %d\n", hpmcounter);
    int counteren;
    switch (readMiscRegNoEffect(MISCREG_PRV)) {
      case PRV_M:
        return true;
      case PRV_S:
        counteren = MISCREG_MCOUNTEREN;
        break;
      case PRV_U:
        counteren = MISCREG_SCOUNTEREN;
        break;
      default:
        panic("Unknown privilege level %d\n", miscRegFile[MISCREG_PRV]);
        return false;
    }
    return (miscRegFile[counteren] & (1ULL << (hpmcounter))) > 0;
}

RegVal
ISA::readMiscRegNoEffect(RegIndex idx) const
{
    // Illegal CSR
    panic_if(idx > NUM_PHYS_MISCREGS, "Illegal CSR index %#x\n", idx);
    DPRINTF(RiscvMisc, "Reading MiscReg %s (%d): %#x.\n",
            MiscRegNames[idx], idx, miscRegFile[idx]);
    return miscRegFile[idx];
}

RegVal
ISA::readMiscReg(RegIndex idx)
{
    switch (idx) {
      case MISCREG_HARTID:
        return tc->contextId();
      case MISCREG_CYCLE:
        if (hpmCounterEnabled(MISCREG_CYCLE)) {
            DPRINTF(RiscvMisc, "Cycle counter at: %llu.\n",
                    tc->getCpuPtr()->curCycle());
            return static_cast<RegVal>(tc->getCpuPtr()->curCycle());
        } else {
            warn("Cycle counter disabled.\n");
            return 0;
        }
      case MISCREG_CYCLEH:
        if (hpmCounterEnabled(MISCREG_CYCLEH)) {
            DPRINTF(RiscvMisc, "Cycle counter at: %llu.\n",
                    tc->getCpuPtr()->curCycle());
            return bits<RegVal>(tc->getCpuPtr()->curCycle(), 63, 32);
        } else {
            warn("Cycle counter disabled.\n");
            return 0;
        }
      case MISCREG_TIME:
        if (hpmCounterEnabled(MISCREG_TIME)) {
            DPRINTF(RiscvMisc, "Wall-clock counter at: %llu.\n",
                    std::time(nullptr));
            return readMiscRegNoEffect(MISCREG_TIME);
        } else {
            warn("Wall clock disabled.\n");
            return 0;
        }
      case MISCREG_TIMEH:
        if (hpmCounterEnabled(MISCREG_TIMEH)) {
            DPRINTF(RiscvMisc, "Wall-clock counter at: %llu.\n",
                    std::time(nullptr));
            return readMiscRegNoEffect(MISCREG_TIMEH);
        } else {
            warn("Wall clock disabled.\n");
            return 0;
        }
      case MISCREG_INSTRET:
        if (hpmCounterEnabled(MISCREG_INSTRET)) {
            DPRINTF(RiscvMisc, "Instruction counter at: %llu.\n",
                    tc->getCpuPtr()->totalInsts());
            return static_cast<RegVal>(tc->getCpuPtr()->totalInsts());
        } else {
            warn("Instruction counter disabled.\n");
            return 0;
        }
      case MISCREG_INSTRETH:
        if (hpmCounterEnabled(MISCREG_INSTRETH)) {
            DPRINTF(RiscvMisc, "Instruction counter at: %llu.\n",
                    tc->getCpuPtr()->totalInsts());
            return bits<RegVal>(tc->getCpuPtr()->totalInsts(), 63, 32);
        } else {
            warn("Instruction counter disabled.\n");
            return 0;
        }
      case MISCREG_IP:
        {
            auto ic = dynamic_cast<RiscvISA::Interrupts *>(
                    tc->getCpuPtr()->getInterruptController(tc->threadId()));
            return ic->readIP();
        }
      case MISCREG_UIP:
        {
            return readMiscReg(MISCREG_IP) & UI_MASK[getPrivilegeModeSet()];
        }
      case MISCREG_SIP:
        {
            return readMiscReg(MISCREG_IP) & SI_MASK[getPrivilegeModeSet()];
        }
      case MISCREG_IE:
        {
            auto ic = dynamic_cast<RiscvISA::Interrupts *>(
                    tc->getCpuPtr()->getInterruptController(tc->threadId()));
            return ic->readIE();
        }
      case MISCREG_UIE:
        {
            return readMiscReg(MISCREG_IE) & UI_MASK[getPrivilegeModeSet()];
        }
      case MISCREG_SIE:
        {
            return readMiscReg(MISCREG_IE) & SI_MASK[getPrivilegeModeSet()];
        }
      case MISCREG_SEPC:
      case MISCREG_MEPC:
        {
            MISA misa = readMiscRegNoEffect(MISCREG_ISA);
            auto val = readMiscRegNoEffect(idx);
            // if compressed instructions are disabled, epc[1] is set to 0
            if (misa.rvc == 0)
                return mbits(val, 63, 2);
            // epc[0] is always 0
            else
                return mbits(val, 63, 1);
        }
      case MISCREG_STATUS:
        {
            // Updating the SD bit.
            // . Per RISC-V ISA Manual, vol II, section 3.1.6.6, page 26,
            // the SD bit is a read-only bit indicating whether any of
            // FS, VS, and XS fields being in the respective dirty state.
            // . Per section 3.1.6, page 20, the SD bit is the most
            // significant bit of the MSTATUS CSR for both RV32 and RV64.
            // . Per section 3.1.6.6, page 29, the explicit formula for
            // updating the SD is,
            //   SD = ((FS==DIRTY) | (XS==DIRTY) | (VS==DIRTY))
            // . Ideally, we want to update the SD after every relevant
            // instruction, however, lazily updating the Status register
            // upon its read produces the same effect as well.
            STATUS status = readMiscRegNoEffect(idx);
            uint64_t sd_bit = \
                (status.xs == 3) || (status.fs == 3) || (status.vs == 3);
            // For RV32, the SD bit is at index 31
            // For RV64, the SD bit is at index 63.
            switch (_rvType) {
                case RV32:
                    status.rv32_sd = sd_bit;
                    break;
                case RV64:
                    status.rv64_sd = sd_bit;
                    break;
                default:
                    panic("%s: Unknown _rvType: %d", name(), (int)_rvType);
            }
            // Check status.mpp
            MISA misa = readMiscRegNoEffect(MISCREG_ISA);
            switch(status.mpp) {
                case PRV_U:
                    status.mpp = (misa.rvu) ? PRV_U : PRV_M;
                    break;
                case PRV_S:
                    if (misa.rvs)
                        status.mpp = PRV_S;
                    else
                        status.mpp = (misa.rvu) ? PRV_U : PRV_M;
                    break;
                case PRV_M:
                    break;
                default:
                    status.mpp = (misa.rvu) ? PRV_U : PRV_M;
            }

            setMiscRegNoEffect(idx, status);

            return readMiscRegNoEffect(idx);
        }
      case MISCREG_USTATUS:
        {
           return readMiscReg(MISCREG_STATUS) &
                  USTATUS_MASKS[rvType()][getPrivilegeModeSet()];
        }
      case MISCREG_SSTATUS:
        {
           return readMiscReg(MISCREG_STATUS) &
                  SSTATUS_MASKS[rvType()][getPrivilegeModeSet()];
        }
      case MISCREG_VLENB:
        {
            return getVecLenInBytes();
        }
      case MISCREG_VTYPE:
        {
            auto rpc = tc->pcState().as<PCState>();
            return rpc.vtype();
        }
      case MISCREG_VL:
        {
            auto rpc = tc->pcState().as<PCState>();
            return (RegVal)rpc.vl();
        }
      case MISCREG_VCSR:
        {
            return readMiscRegNoEffect(MISCREG_VXSAT) |
                  (readMiscRegNoEffect(MISCREG_VXRM) << 1);
        }
        break;
      case MISCREG_FFLAGS_EXE:
        {
            return readMiscRegNoEffect(MISCREG_FFLAGS) & FFLAGS_MASK;
        }
      case MISCREG_FCSR:
        {
            return readMiscRegNoEffect(MISCREG_FFLAGS) |
                  (readMiscRegNoEffect(MISCREG_FRM) << FRM_OFFSET);
        }
      case MISCREG_TMDATA:
        {
            TMDATA tmdata = readMiscRegNoEffect(idx);
            return (tmdata.tile_c_datatype << 16) |
                   (tmdata.tile_b_datatype << 8) |
                   tmdata.tile_a_datatype;
        }
      case MISCREG_TMSIZE:
        {
            TMSIZE tmsize = readMiscRegNoEffect(idx);
            return (tmsize.k << 20) |
                   (tmsize.n << 10) |
                   tmsize.m;
        }
      default:
        // Try reading HPM counters
        // As a placeholder, all HPM counters are just cycle counters
        if (idx >= MISCREG_HPMCOUNTER03 &&
                idx <= MISCREG_HPMCOUNTER31) {
            if (hpmCounterEnabled(idx)) {
                DPRINTF(RiscvMisc, "HPM counter %d: %llu.\n",
                        idx - MISCREG_CYCLE, tc->getCpuPtr()->curCycle());
                return tc->getCpuPtr()->curCycle();
            } else {
                warn("HPM counter %d disabled.\n", idx - MISCREG_CYCLE);
                return 0;
            }
        } else if (idx >= MISCREG_HPMCOUNTER03H &&
                idx <= MISCREG_HPMCOUNTER31H) {
            if (hpmCounterEnabled(idx)) {
                DPRINTF(RiscvMisc, "HPM counter %d: %llu.\n",
                        idx - MISCREG_CYCLE, tc->getCpuPtr()->curCycle());
                return bits<RegVal>(tc->getCpuPtr()->curCycle(), 63, 32);
            } else {
                warn("HPM counter %d disabled.\n", idx - MISCREG_CYCLE);
                return 0;
            }
        }
        return readMiscRegNoEffect(idx);
    }
}

void
ISA::setMiscRegNoEffect(RegIndex idx, RegVal val)
{
    // Illegal CSR
    panic_if(idx > NUM_PHYS_MISCREGS, "Illegal CSR index %#x\n", idx);
    DPRINTF(RiscvMisc, "Setting MiscReg %s (%d) to %#x.\n",
            MiscRegNames[idx], idx, val);
    miscRegFile[idx] = val;
}

void
ISA::setMiscReg(RegIndex idx, RegVal val)
{
    if (idx >= MISCREG_CYCLE && idx <= MISCREG_HPMCOUNTER31) {
        // Ignore writes to HPM counters for now
        warn("Ignoring write to miscreg %s.\n", MiscRegNames[idx]);
    } else {
        switch (idx) {

          // From section 3.7.1 of RISCV priv. specs
          // V1.12, the odd-numbered configuration
          // registers are illegal for RV64 and
          // each 64 bit CFG register hold configurations
          // for 8 PMP entries.

          case MISCREG_PMPCFG0:
          case MISCREG_PMPCFG1:
          case MISCREG_PMPCFG2:
          case MISCREG_PMPCFG3:
            {
                // PMP registers should only be modified in M mode
                assert(readMiscRegNoEffect(MISCREG_PRV) == PRV_M);

                int regSize = 0;
                switch (_rvType) {
                    case RV32:
                        regSize = 4;
                    break;
                    case RV64:
                        regSize = 8;
                    break;
                    default:
                        panic("%s: Unknown _rvType: %d", name(), (int)_rvType);
                }

                // Specs do not seem to mention what should be
                // configured first, cfg or address regs!
                // qemu seems to update the tables when
                // pmp addr regs are written (with the assumption
                // that cfg regs are already written)
                RegVal res = 0;
                RegVal old_val = readMiscRegNoEffect(idx);

                for (int i=0; i < regSize; i++) {

                    uint8_t cfg_val = (val >> (8*i)) & 0xff;
                    auto mmu = dynamic_cast<RiscvISA::MMU *>
                                (tc->getMMUPtr());

                    // Form pmp_index using the index i and
                    // PMPCFG register number
                    uint32_t pmp_index = i+(4*(idx-MISCREG_PMPCFG0));
                    bool result = mmu->getPMP()->pmpUpdateCfg(pmp_index,cfg_val);
                    if (result) {
                        res |= ((RegVal)cfg_val << (8*i));
                    } else {
                        res |= (old_val & (0xFF << (8*i)));
                    }
                }

                setMiscRegNoEffect(idx, res);
            }
            break;
          case MISCREG_PMPADDR00 ... MISCREG_PMPADDR15:
            {
                // PMP registers should only be modified in M mode
                assert(readMiscRegNoEffect(MISCREG_PRV) == PRV_M);

                auto mmu = dynamic_cast<RiscvISA::MMU *>
                              (tc->getMMUPtr());
                uint32_t pmp_index = idx-MISCREG_PMPADDR00;
                if (mmu->getPMP()->pmpUpdateAddr(pmp_index, val)) {
                    setMiscRegNoEffect(idx, val);
                }
            }
            break;

          case MISCREG_IP:
            {
                val = val & MI_MASK[getPrivilegeModeSet()];
                auto ic = dynamic_cast<RiscvISA::Interrupts *>(
                    tc->getCpuPtr()->getInterruptController(tc->threadId()));
                ic->setIP(val);
            }
            break;
          case MISCREG_UIP:
            {
                RegVal mask = UI_MASK[getPrivilegeModeSet()];
                val = (val & mask) | (readMiscReg(MISCREG_IP) & ~mask);
                setMiscReg(MISCREG_IP, val);
            }
            break;
          case MISCREG_SIP:
            {
                RegVal mask = SI_MASK[getPrivilegeModeSet()];
                val = (val & mask) | (readMiscReg(MISCREG_IP) & ~mask);
                setMiscReg(MISCREG_IP, val);
            }
            break;
          case MISCREG_IE:
            {
                val = val & MI_MASK[getPrivilegeModeSet()];
                auto ic = dynamic_cast<RiscvISA::Interrupts *>(
                    tc->getCpuPtr()->getInterruptController(tc->threadId()));
                ic->setIE(val);
            }
            break;
          case MISCREG_UIE:
            {
                RegVal mask = UI_MASK[getPrivilegeModeSet()];
                val = (val & mask) | (readMiscReg(MISCREG_IE) & ~mask);
                setMiscReg(MISCREG_IE, val);
            }
            break;
          case MISCREG_SIE:
            {
                RegVal mask = SI_MASK[getPrivilegeModeSet()];
                val = (val & mask) | (readMiscReg(MISCREG_IE) & ~mask);
                setMiscReg(MISCREG_IE, val);
            }
            break;
          case MISCREG_SATP:
            {
                // we only support bare and Sv39 mode; setting a different mode
                // shall have no effect (see 4.1.12 in priv ISA manual)
                SATP cur_val = readMiscRegNoEffect(idx);
                SATP new_val = val;
                if (new_val.mode != AddrXlateMode::BARE &&
                    new_val.mode != AddrXlateMode::SV39)
                    new_val.mode = cur_val.mode;
                setMiscRegNoEffect(idx, new_val);
            }
            break;
          case MISCREG_SENVCFG:
            {
                // panic on write to bitfields that aren't implemented in gem5
                SENVCFG panic_mask = 0;
                panic_mask.pmm = 3;

                SENVCFG wpri_mask = 0;
                wpri_mask.wpri_1 = ~wpri_mask.wpri_1;
                wpri_mask.wpri_2 = ~wpri_mask.wpri_2;
                wpri_mask.wpri_3 = ~wpri_mask.wpri_3;

                if ((panic_mask & val) != 0) {
                    panic("Tried to write to an unimplemented bitfield in the "
                    "senvcfg CSR!\nThe attempted write was:\n %" PRIu64 "\n",
                    val);
                }

                setMiscRegNoEffect(idx, val & ~wpri_mask);
            }
            break;
          case MISCREG_TSELECT:
            {
                // we don't support debugging, so always set a different value
                // than written
                setMiscRegNoEffect(idx, val + 1);
            }
            break;
          case MISCREG_ISA:
            {
                MISA cur_misa = (MISA)readMiscRegNoEffect(MISCREG_ISA);
                MISA new_misa = (MISA)val;
                // only allow to disable compressed instructions
                // if the following instruction is 4-byte aligned
                if (new_misa.rvc == 0 &&
                        bits(tc->pcState().as<RiscvISA::PCState>().npc(),
                            2, 0) != 0) {
                    new_misa.rvc = new_misa.rvc | cur_misa.rvc;
                }
                if (!getEnableRvv()) {
                    new_misa.rvv = 0;
                }
                new_misa.rvn = cur_misa.rvn;
                new_misa.rvs = cur_misa.rvs;
                new_misa.rvu = cur_misa.rvu;
                setMiscRegNoEffect(idx, new_misa);
            }
            break;
          case MISCREG_STATUS:
            {
                val = val & MSTATUS_MASKS[rvType()][getPrivilegeModeSet()];
                if (_rvType != RV32) {
                    // SXL and UXL are hard-wired to 64 bit
                    auto cur = readMiscRegNoEffect(idx);
                    val &= ~(STATUS_SXL_MASK | STATUS_UXL_MASK);
                    val |= cur & (STATUS_SXL_MASK | STATUS_UXL_MASK);
                }
                if (!getEnableRvv()) {
                    // Always OFF is rvv is disabled.
                    val &= ~STATUS_VS_MASK;
                }
                setMiscRegNoEffect(idx, val);
            }
            break;
          case MISCREG_USTATUS:
            {
                RegVal mask = USTATUS_MASKS[rvType()][getPrivilegeModeSet()];
                val = (val & mask) |
                      (readMiscRegNoEffect(MISCREG_STATUS) & ~mask);
                setMiscReg(MISCREG_STATUS, val);
            }
            break;
          case MISCREG_SSTATUS:
            {
                RegVal mask = SSTATUS_MASKS[rvType()][getPrivilegeModeSet()];
                val = (val & mask) |
                      (readMiscRegNoEffect(MISCREG_STATUS) & ~mask);
                setMiscReg(MISCREG_STATUS, val);
            }
            break;
          case MISCREG_VXSAT:
            {
                setMiscRegNoEffect(idx, val & 0x1);
            }
            break;
          case MISCREG_VXRM:
            {
                setMiscRegNoEffect(idx, val & 0x3);
            }
            break;
          case MISCREG_VCSR:
            {
                setMiscRegNoEffect(MISCREG_VXSAT, val & 0x1);
                setMiscRegNoEffect(MISCREG_VXRM, (val & 0x6) >> 1);
            }
            break;
          case MISCREG_FFLAGS_EXE:
            {
                RegVal new_val = readMiscRegNoEffect(MISCREG_FFLAGS);
                new_val |= (val & FFLAGS_MASK);
                setMiscRegNoEffect(MISCREG_FFLAGS, new_val);
            }
            break;
          case MISCREG_FFLAGS:
            {
                setMiscRegNoEffect(MISCREG_FFLAGS, val & FFLAGS_MASK);
            }
            break;
          case MISCREG_FRM:
            {
                setMiscRegNoEffect(MISCREG_FRM, val & FRM_MASK);
            }
            break;
          case MISCREG_FCSR:
            {
                setMiscRegNoEffect(MISCREG_FFLAGS, bits(val, 4, 0));
                setMiscRegNoEffect(MISCREG_FRM, bits(val, 7, 5));
            }
            break;
          case MISCREG_TMDATA:
            {
                TMDATA tmdata = readMiscRegNoEffect(idx);
                tmdata.tile_c_datatype = bits(val, 23, 16);
                tmdata.tile_b_datatype = bits(val, 15, 8);
                tmdata.tile_a_datatype = bits(val, 7, 0);
                setMiscRegNoEffect(idx, tmdata);
            }
            break;
          case MISCREG_TMSIZE:
            {
                TMSIZE tmsize = readMiscRegNoEffect(idx);
                tmsize.k = bits(val, 29, 20);
                tmsize.n = bits(val, 19, 10);
                tmsize.m = bits(val, 9, 0);
                setMiscRegNoEffect(idx, tmsize);
            }
            break;
          default:
            setMiscRegNoEffect(idx, val);
        }
    }
}

void
ISA::serialize(CheckpointOut &cp) const
{
    BaseISA::serialize(cp);

    DPRINTF(Checkpoint, "Serializing Riscv Misc Registers\n");
    SERIALIZE_CONTAINER(miscRegFile);
}

void
ISA::unserialize(CheckpointIn &cp)
{
    DPRINTF(Checkpoint, "Unserializing Riscv Misc Registers\n");
    UNSERIALIZE_CONTAINER(miscRegFile);
}

void
ISA::handleLockedSnoop(PacketPtr pkt, Addr cacheBlockMask)
{
    Addr& load_reservation_addr = load_reservation_addrs[tc->contextId()];

    if (load_reservation_addr == INVALID_RESERVATION_ADDR)
        return;
    Addr snoop_addr = pkt->getAddr() & cacheBlockMask;
    DPRINTF(LLSC, "Locked snoop on address %x.\n", snoop_addr);
    if ((load_reservation_addr & cacheBlockMask) == snoop_addr)
        load_reservation_addr = INVALID_RESERVATION_ADDR;
}


void
ISA::handleLockedRead(const RequestPtr &req)
{
    Addr& load_reservation_addr = load_reservation_addrs[tc->contextId()];

    load_reservation_addr = req->getPaddr();
    DPRINTF(LLSC, "[cid:%d]: Reserved address %x.\n",
            req->contextId(), req->getPaddr());
}

bool
ISA::handleLockedWrite(const RequestPtr &req, Addr cacheBlockMask)
{
    Addr& load_reservation_addr = load_reservation_addrs[tc->contextId()];
    bool lr_addr_empty = (load_reservation_addr == INVALID_RESERVATION_ADDR);

    // Normally RISC-V uses zero to indicate success and nonzero to indicate
    // failure (right now only 1 is reserved), but in gem5 zero indicates
    // failure and one indicates success, so here we conform to that (it should
    // be switched in the instruction's implementation)

    DPRINTF(LLSC, "[cid:%d]: load_reservation_addrs empty? %s.\n",
            req->contextId(),
            lr_addr_empty ? "yes" : "no");
    if (!lr_addr_empty) {
        DPRINTF(LLSC, "[cid:%d]: addr = %x.\n", req->contextId(),
                req->getPaddr() & cacheBlockMask);
        DPRINTF(LLSC, "[cid:%d]: last locked addr = %x.\n", req->contextId(),
                load_reservation_addr & cacheBlockMask);
    }
    if (lr_addr_empty ||
            (load_reservation_addr & cacheBlockMask)
            != ((req->getPaddr() & cacheBlockMask))) {
        req->setExtraData(0);
        int stCondFailures = tc->readStCondFailures();
        tc->setStCondFailures(++stCondFailures);
        if (stCondFailures % WARN_FAILURE == 0) {
            warn("%i: context %d: %d consecutive SC failures.\n",
                    curTick(), tc->contextId(), stCondFailures);
        }

        // Must clear any reservations
        load_reservation_addr = INVALID_RESERVATION_ADDR;

        return false;
    }
    if (req->isUncacheable()) {
        req->setExtraData(2);
    }

    // Must clear any reservations
    load_reservation_addr = INVALID_RESERVATION_ADDR;

    DPRINTF(LLSC, "[cid:%d]: SC success! Current locked addr = %x.\n",
            req->contextId(), load_reservation_addr & cacheBlockMask);
    return true;
}

void
ISA::globalClearExclusive()
{
    tc->getCpuPtr()->wakeup(tc->threadId());
    Addr& load_reservation_addr = load_reservation_addrs[tc->contextId()];
    load_reservation_addr = INVALID_RESERVATION_ADDR;
}

void
ISA::resetThread()
{
    Reset().invoke(tc);
}

Addr
ISA::getFaultHandlerAddr(RegIndex idx, uint64_t cause, bool intr) const
{
    auto vec = tc->readMiscRegNoEffect(idx);
    Addr addr = mbits(vec, 63, 2);
    if (intr && bits(vec, 1, 0) == 1)
        addr += 4 * cause;
    return addr;
}

} // namespace RiscvISA
} // namespace gem5

std::ostream &
operator<<(std::ostream &os, gem5::RiscvISA::PrivilegeMode pm)
{
    switch (pm) {
    case gem5::RiscvISA::PRV_U:
        return os << "PRV_U";
    case gem5::RiscvISA::PRV_S:
        return os << "PRV_S";
    case gem5::RiscvISA::PRV_M:
        return os << "PRV_M";
    }
    return os << "PRV_<invalid>";
}
