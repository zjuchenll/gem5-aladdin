/*
 * Copyright (c) 2003 The Regents of The University of Michigan
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

#include <string>

#include "cpu/base_cpu.hh"
#include "cpu/exec_context.hh"

#ifdef FULL_SYSTEM
#include "sim/system.hh"
#else
#include "sim/prog.hh"
#endif

using namespace std;

// constructor
#ifdef FULL_SYSTEM
ExecContext::ExecContext(BaseCPU *_cpu, int _thread_num, System *_sys,
                         AlphaItb *_itb, AlphaDtb *_dtb,
                         FunctionalMemory *_mem)
    : kernelStats(this, _cpu), cpu(_cpu), thread_num(_thread_num),
      cpu_id(-1), mem(_mem), itb(_itb), dtb(_dtb), system(_sys),
      memCtrl(_sys->memCtrl), physmem(_sys->physmem),
      func_exe_insn(0), storeCondFailures(0)
{
    memset(&regs, 0, sizeof(RegFile));
    setStatus(ExecContext::Unallocated);
}
#else
ExecContext::ExecContext(BaseCPU *_cpu, int _thread_num,
                         Process *_process, int _asid)
    : cpu(_cpu), thread_num(_thread_num), cpu_id(-1),
      process(_process), asid (_asid),
      func_exe_insn(0), storeCondFailures(0)
{
    setStatus(ExecContext::Unallocated);

    mem = process->getMemory();
}

ExecContext::ExecContext(BaseCPU *_cpu, int _thread_num,
                         FunctionalMemory *_mem, int _asid)
    : cpu(_cpu), thread_num(_thread_num), process(0), mem(_mem), asid(_asid),
      func_exe_insn(0), storeCondFailures(0)
{
}
#endif


void
ExecContext::takeOverFrom(ExecContext *oldContext)
{
    // some things should already be set up
    assert(mem == oldContext->mem);
#ifdef FULL_SYSTEM
    assert(system == oldContext->system);
#else
    assert(process == oldContext->process);
#endif

    // copy over functional state
    _status = oldContext->_status;
#ifdef FULL_SYSTEM
    kernelStats = oldContext->kernelStats;
#endif
    regs = oldContext->regs;
    cpu_id = oldContext->cpu_id;
    func_exe_insn = oldContext->func_exe_insn;

    storeCondFailures = 0;

    oldContext->_status = ExecContext::Unallocated;
}


void
ExecContext::setStatus(Status new_status)
{
#ifdef FULL_SYSTEM
    if (status() == new_status)
        return;

    // Don't change the status from active if there are pending interrupts
    if (new_status == Suspended && cpu->check_interrupts()) {
        assert(status() == Active);
        return;
    }
#endif

    _status = new_status;
    cpu->execCtxStatusChg();
}

void
ExecContext::regStats(const string &name)
{
#ifdef FULL_SYSTEM
    kernelStats.regStats(name + ".kern");
#endif
}
