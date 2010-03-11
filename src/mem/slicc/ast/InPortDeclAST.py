# Copyright (c) 1999-2008 Mark D. Hill and David A. Wood
# Copyright (c) 2009 The Hewlett-Packard Development Company
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from slicc.ast.DeclAST import DeclAST
from slicc.ast.TypeAST import TypeAST
from slicc.symbols import Func, Type, Var

class InPortDeclAST(DeclAST):
    def __init__(self, slicc, ident, msg_type, var_expr, pairs, statements):
        super(InPortDeclAST, self).__init__(slicc, pairs)

        self.ident = ident
        self.msg_type = msg_type
        self.var_expr = var_expr
        self.statements = statements
        self.queue_type = TypeAST(slicc, "InPort")

    def __repr__(self):
        return "[InPortDecl: %s]" % self.ident

    def generate(self):
        symtab = self.symtab
        void_type = symtab.find("void", Type)

        code = self.slicc.codeFormatter()
        queue_type = self.var_expr.generate(code)
        if not queue_type.isInPort:
            self.error("The inport queue's type must have the 'inport' " + \
                       "attribute.  Type '%s' does not have this attribute.",
                       queue_type)

        type = self.queue_type.type
        in_port = Var(self.symtab, self.ident, self.location, type, str(code),
                      self.pairs)
        symtab.newSymbol(in_port)

        symtab.pushFrame()
        param_types = []

        # Check for Event
        type = symtab.find("Event", Type)
        if type is None:
            self.error("in_port decls require 'Event' enumeration defined")
        param_types.append(type)

        # Check for Address
        type = symtab.find("Address", Type)
        if type is None:
            self.error("in_port decls require 'Address' type to be defined")

        param_types.append(type)

        # Add the trigger method - FIXME, this is a bit dirty
        pairs = { "external" : "yes" }
        func = Func(self.symtab, "trigger", self.location, void_type,
                    param_types, [], "", pairs, None)
        symtab.newSymbol(func)

        param_types = []
        # Check for Event2
        type = symtab.find("Event", Type)
        if type is None:
            self.error("in_port decls require 'Event' enumeration")

        param_types.append(type)

        # Check for Address2
        type = symtab.find("Address", Type)
        if type is None:
            self.error("in_port decls require 'Address' type to be defined")

        param_types.append(type)

        # Add the doubleTrigger method - this hack supports tiggering
        # two simulateous events
        #
        # The key is that the second transistion cannot fail because
        # the first event cannot be undone therefore you must do some
        # checks before calling double trigger to ensure that won't
        # happen
        func = Func(self.symtab, "doubleTrigger", self.location, void_type,
                    param_types, [], "", pairs, None)
        symtab.newSymbol(func)

        # Add the continueProcessing method - this hack supports
        # messages that don't trigger events
        func = Func(self.symtab, "continueProcessing", self.location,
                    void_type, [], [], "", pairs, None)
        symtab.newSymbol(func)

        if self.statements is not None:
            rcode = self.slicc.codeFormatter()
            rcode.indent()
            rcode.indent()
            self.statements.generate(rcode, None)
            in_port["c_code_in_port"] = str(rcode)
        symtab.popFrame()

        # Add port to state machine
        machine = symtab.state_machine
        if machine is None:
            self.error("InPort declaration not part of a machine.")

        machine.addInPort(in_port)
