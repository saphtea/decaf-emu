#pragma once
#include <map>
#include <asmjit/asmjit.h>
#include "cpu/cpu.h"

namespace cpu
{

namespace jit
{

static const bool JIT_CONTINUE_ON_ERROR = false;
static const int JIT_MAX_INST = 20000;

/*
Register Assignments:
RAX . Scratch
RCX . Scratch
RDX . Scratch
RDI . Scratch
RSI . mem::base()
RBX . ThreadState*
RBP .
RSP . Emu Stack Pointer.
R8-R15 . PPCGPR Storage
*/

class PPCEmuAssembler : public asmjit::X86Assembler
{
private:
   class ErrorHandler : public asmjit::ErrorHandler
   {
   public:
      bool handleError(asmjit::Error code, const char* message) override;
   } errHandler;

public:
   PPCEmuAssembler(asmjit::Runtime* runtime) :
      asmjit::X86Assembler(runtime, asmjit::kArchX64)
   {
      setErrorHandler(&errHandler);

      eax = zax.r32();
      ecx = zcx.r32();
      edx = zdx.r32();
      r8d = asmjit::x86::r8d;
      r9d = asmjit::x86::r9d;

#define PPCTSReg(mm) asmjit::X86Mem(zbx, (int32_t)offsetof(ThreadState, mm), sizeof(ThreadState::mm))
      for (auto i = 0; i < 32; ++i) {
         ppcgpr[i] = PPCTSReg(gpr[i]);
      }

      for (auto i = 0; i < 32; ++i) {
         ppcfpr[i] = PPCTSReg(fpr[i]);
         ppcfprps[i][0] = PPCTSReg(fpr[i].paired0);
         ppcfprps[i][1] = PPCTSReg(fpr[i].paired1);
      }

      ppccr = PPCTSReg(cr);
      ppcxer = PPCTSReg(xer.value);
      ppclr = PPCTSReg(lr);
      ppcctr = PPCTSReg(ctr);
      ppcfpscr = PPCTSReg(fpscr);

      for (auto i = 0; i < 8; ++i) {
         ppcgqr[i] = PPCTSReg(gqr[i].value);
      }

      ppcreserve = PPCTSReg(reserve);
      ppcreserveAddress = PPCTSReg(reserveAddress);
      ppcreserveData = PPCTSReg(reserveData);
#undef PPCTSReg

      state = zbx;
      membase = zsi;
      cia = zdi;

      xmm0 = asmjit::x86::xmm0;
      xmm1 = asmjit::x86::xmm1;
   }

   void shiftTo(asmjit::X86GpReg reg, int s, int d)
   {
      if (s > d) {
         shr(reg, s - d);
      } else if (d > s) {
         shl(reg, d - s);
      }
   }

   asmjit::X86GpReg state;
   asmjit::X86GpReg membase;
   asmjit::X86GpReg cia;

   asmjit::X86GpReg eax;
   asmjit::X86GpReg ecx;
   asmjit::X86GpReg edx;
   asmjit::X86GpReg r8d;
   asmjit::X86GpReg r9d;

   asmjit::X86XmmReg xmm0;
   asmjit::X86XmmReg xmm1;

   asmjit::X86Mem ppcgpr[32];
   asmjit::X86Mem ppcfpr[32];
   asmjit::X86Mem ppcfprps[32][2];
   asmjit::X86Mem ppccr;
   asmjit::X86Mem ppcxer;
   asmjit::X86Mem ppclr;
   asmjit::X86Mem ppcctr;
   asmjit::X86Mem ppcfpscr;
   asmjit::X86Mem ppcgqr[8];

   asmjit::X86Mem ppcreserve;
   asmjit::X86Mem ppcreserveAddress;
   asmjit::X86Mem ppcreserveData;
};

template<typename T, typename Z>
T asmjit_cast(Z* base, size_t offset = 0)
{
   return reinterpret_cast<T>(((char*)base) + offset);
}

typedef void* JitCode;
typedef void* JitFinale;
typedef uint32_t(*JitCall)(ThreadState*, JitCode);

typedef std::map<uint32_t, asmjit::Label> JumpLabelMap;

extern JitCall gCallFn;
extern JitFinale gFinaleFn;

struct JitBlock
{
   JitBlock(uint32_t _start) {
      start = _start;
      end = _start;
      entry = nullptr;
   }

   uint32_t start;
   uint32_t end;

   JitCode entry;
   std::map<uint32_t, JitCode> targets;
};

} // namespace jit

} // namespace cpu
