// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_INTERPRETER_BYTECODE_PIPELINE_H_
#define V8_INTERPRETER_BYTECODE_PIPELINE_H_

#include "src/interpreter/bytecode-register-allocator.h"
#include "src/interpreter/bytecodes.h"
#include "src/zone-containers.h"

namespace v8 {
namespace internal {
namespace interpreter {

class BytecodeNode;
class BytecodeSourceInfo;

// Interface for bytecode pipeline stages.
class BytecodePipelineStage {
 public:
  virtual ~BytecodePipelineStage() {}

  // Write bytecode node |node| into pipeline. The node is only valid
  // for the duration of the call. Callee's should clone it if
  // deferring Write() to the next stage.
  virtual void Write(BytecodeNode* node) = 0;

  // Flush state for bytecode array offset calculation. Returns the
  // current size of bytecode array.
  virtual size_t FlushForOffset() = 0;

  // Flush state to terminate basic block.
  virtual void FlushBasicBlock() = 0;
};

// Source code position information.
class BytecodeSourceInfo final {
 public:
  static const int kUninitializedPosition = -1;

  BytecodeSourceInfo(int position = kUninitializedPosition,
                     bool is_statement = false)
      : source_position_(position), is_statement_(is_statement) {}

  // Combine later source info with current.
  void Update(const BytecodeSourceInfo& entry);

  int source_position() const {
    DCHECK(is_valid());
    return source_position_;
  }

  bool is_statement() const { return is_valid() && is_statement_; }
  bool is_expression() const { return is_valid() && !is_statement_; }

  bool is_valid() const { return source_position_ != kUninitializedPosition; }
  void set_invalid() { source_position_ = kUninitializedPosition; }

  bool operator==(const BytecodeSourceInfo& other) const {
    return source_position_ == other.source_position_ &&
           is_statement_ == other.is_statement_;
  }
  bool operator!=(const BytecodeSourceInfo& other) const {
    return source_position_ != other.source_position_ ||
           is_statement_ != other.is_statement_;
  }

 private:
  int source_position_;
  bool is_statement_;

  DISALLOW_COPY_AND_ASSIGN(BytecodeSourceInfo);
};

// A container for a generated bytecode, it's operands, and source information.
// These must be allocated by a BytecodeNodeAllocator instance.
class BytecodeNode final : ZoneObject {
 public:
  explicit BytecodeNode(Bytecode bytecode = Bytecode::kIllegal);
  BytecodeNode(Bytecode bytecode, uint32_t operand0,
               OperandScale operand_scale);
  BytecodeNode(Bytecode bytecode, uint32_t operand0, uint32_t operand1,
               OperandScale operand_scale);
  BytecodeNode(Bytecode bytecode, uint32_t operand0, uint32_t operand1,
               uint32_t operand2, OperandScale operand_scale);
  BytecodeNode(Bytecode bytecode, uint32_t operand0, uint32_t operand1,
               uint32_t operand2, uint32_t operand3,
               OperandScale operand_scale);

  BytecodeNode(const BytecodeNode& other);
  BytecodeNode& operator=(const BytecodeNode& other);

  void set_bytecode(Bytecode bytecode);
  void set_bytecode(Bytecode bytecode, uint32_t operand0,
                    OperandScale operand_scale);

  // Clone |other|.
  void Clone(const BytecodeNode* const other);

  // Print to stream |os|.
  void Print(std::ostream& os) const;

  // Return the size when this node is serialized to a bytecode array.
  size_t Size() const;

  // Transform to a node representing |new_bytecode| which has one
  // operand more than the current bytecode.
  void Transform(Bytecode new_bytecode, uint32_t extra_operand,
                 OperandScale extra_operand_scale);

  Bytecode bytecode() const { return bytecode_; }

  uint32_t operand(int i) const {
    DCHECK_LT(i, operand_count());
    return operands_[i];
  }
  uint32_t* operands() { return operands_; }
  const uint32_t* operands() const { return operands_; }

  int operand_count() const { return Bytecodes::NumberOfOperands(bytecode_); }
  OperandScale operand_scale() const { return operand_scale_; }
  void set_operand_scale(OperandScale operand_scale) {
    operand_scale_ = operand_scale;
  }

  const BytecodeSourceInfo& source_info() const { return source_info_; }
  BytecodeSourceInfo& source_info() { return source_info_; }

  bool operator==(const BytecodeNode& other) const;
  bool operator!=(const BytecodeNode& other) const { return !(*this == other); }

 private:
  static const int kInvalidPosition = kMinInt;
  static const size_t kMaxOperands = 4;

  Bytecode bytecode_;
  uint32_t operands_[kMaxOperands];
  OperandScale operand_scale_;
  BytecodeSourceInfo source_info_;
};

std::ostream& operator<<(std::ostream& os, const BytecodeSourceInfo& info);
std::ostream& operator<<(std::ostream& os, const BytecodeNode& node);

}  // namespace interpreter
}  // namespace internal
}  // namespace v8

#endif  // V8_INTERPRETER_BYTECODE_PIPELINE_H_
