/*
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
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

#ifndef __FAULTS_HH__
#define __FAULTS_HH__

#include "base/refcnt.hh"
#include "sim/stats.hh"
#include "config/full_system.hh"

class ExecContext;
class FaultBase;
typedef RefCountingPtr<FaultBase> Fault;

typedef const char * FaultName;
typedef Stats::Scalar<> FaultStat;

// Each class has it's name statically define in _name,
// and has a virtual function to access it's name.
// The function is necessary because otherwise, all objects
// which are being accessed cast as a FaultBase * (namely
// all faults returned using the Fault type) will use the
// generic FaultBase name.

class FaultBase : public RefCounted
{
  public:
    virtual FaultName name() = 0;
    virtual FaultStat & stat() = 0;
#if FULL_SYSTEM
    virtual void invoke(ExecContext * xc) = 0;
#else
    virtual void invoke(ExecContext * xc);
#endif
//    template<typename T>
//    bool isA() {return dynamic_cast<T *>(this);}
    virtual bool isMachineCheckFault() {return false;}
    virtual bool isAlignmentFault() {return false;}
};

FaultBase * const NoFault = 0;

//The ISAs are each responsible for providing a genMachineCheckFault and a
//genAlignmentFault functions, which return faults to use in the case of a
//machine check fault or an alignment fault, respectively. Base classes which
//provide the name() function, and the isMachineCheckFault and isAlignmentFault
//functions are provided below.

class MachineCheckFault : public virtual FaultBase
{
  private:
    static FaultName _name;
  public:
    FaultName name() {return _name;}
    bool isMachineCheckFault() {return true;}
};

class AlignmentFault : public virtual FaultBase
{
  private:
    static FaultName _name;
  public:
    FaultName name() {return _name;}
    bool isAlignmentFault() {return true;}
};


#endif // __FAULTS_HH__
