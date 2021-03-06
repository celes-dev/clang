//==-- loop_proto_to_cxx.cpp - Protobuf-C++ conversion ---------------------==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements functions for converting between protobufs and C++. Differs from
// proto_to_cxx.cpp by wrapping all the generated C++ code in a single for
// loop. Also coutputs a different function signature that includes a
// size_t parameter for the loop to use. The C++ code generated is meant to
// stress the LLVM loop vectorizer.
//
// Still a work in progress.
//
//===----------------------------------------------------------------------===//

#include "cxx_loop_proto.pb.h"
#include "proto_to_cxx.h"

// The following is needed to convert protos in human-readable form
#include <google/protobuf/text_format.h>

#include <ostream>
#include <sstream>

namespace clang_fuzzer {

// Forward decls.
std::ostream &operator<<(std::ostream &os, const BinaryOp &x);
std::ostream &operator<<(std::ostream &os, const StatementSeq &x);

// Proto to C++.
std::ostream &operator<<(std::ostream &os, const Const &x) {
  return os << "(" << x.val() << ")";
}
std::ostream &operator<<(std::ostream &os, const VarRef &x) {
  switch (x.arr()) {
    case VarRef::ARR_A:
      return os << "a[i]";
    case VarRef::ARR_B:
      return os << "b[i]";
    case VarRef::ARR_C:
      return os << "c[i]";
  }
}
std::ostream &operator<<(std::ostream &os, const Rvalue &x) {
  if (x.has_cons())
    return os << x.cons();
  if (x.has_binop())
    return os << x.binop();
  if (x.has_varref())
    return os << x.varref();
  return os << "1";
}
std::ostream &operator<<(std::ostream &os, const BinaryOp &x) {
  os << "(" << x.left();
  switch (x.op()) {
  case BinaryOp::PLUS:
    os << "+";
    break;
  case BinaryOp::MINUS:
    os << "-";
    break;
  case BinaryOp::MUL:
    os << "*";
    break;
  case BinaryOp::DIV:
    os << "/";
    break;
  case BinaryOp::MOD:
    os << "%";
    break;
  case BinaryOp::XOR:
    os << "^";
    break;
  case BinaryOp::AND:
    os << "&";
    break;
  case BinaryOp::OR:
    os << "|";
    break;
  case BinaryOp::EQ:
    os << "==";
    break;
  case BinaryOp::NE:
    os << "!=";
    break;
  case BinaryOp::LE:
    os << "<=";
    break;
  case BinaryOp::GE:
    os << ">=";
    break;
  case BinaryOp::LT:
    os << "<";
    break;
  case BinaryOp::GT:
    os << ">";
    break;
  }
  return os << x.right() << ")";
}
std::ostream &operator<<(std::ostream &os, const AssignmentStatement &x) {
  return os << x.varref() << "=" << x.rvalue() << ";\n";
}
std::ostream &operator<<(std::ostream &os, const IfElse &x) {
  return os << "if (" << x.cond() << "){\n"
            << x.if_body() << "} else { \n"
            << x.else_body() << "}\n";
}
std::ostream &operator<<(std::ostream &os, const Statement &x) {
  return os << x.assignment();
}
std::ostream &operator<<(std::ostream &os, const StatementSeq &x) {
  for (auto &st : x.statements())
    os << st;
  return os;
}
std::ostream &operator<<(std::ostream &os, const LoopFunction &x) {
  return os << "void foo(int *a, int *b, int *__restrict__ c, size_t s) {\n"
            << "for (int i=0; i<s; i++){\n"
            << x.statements() << "}\n}\n";
}

// ---------------------------------

std::string LoopFunctionToString(const LoopFunction &input) {
  std::ostringstream os;
  os << input;
  return os.str();
}
std::string LoopProtoToCxx(const uint8_t *data, size_t size) {
  LoopFunction message;
  if (!message.ParsePartialFromArray(data, size))
    return "#error invalid proto\n";
  return LoopFunctionToString(message);
}

} // namespace clang_fuzzer
