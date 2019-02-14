#pragma once

#include "directed_graph.h"
#include "utils.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;
using namespace std;

namespace DHLS {

  enum ExecutionActionType {
    EXECUTION_ACTION_INSTRUCTION,
    EXECUTION_ACTION_TAG,
    EXECUTION_ACTION_BASIC_BLOCK
  };

  class ExecutionAction {
    llvm::Instruction* instr;
    std::string tag;
    llvm::BasicBlock* bb;
    ExecutionActionType tp;
    
  public:

    ExecutionAction() :
      instr(nullptr), tag(""), bb(nullptr), tp(EXECUTION_ACTION_TAG) {}

    ExecutionAction(Instruction* const instr_) :
      instr(instr_), tag(""), bb(nullptr), tp(EXECUTION_ACTION_INSTRUCTION) {}

    ExecutionAction(const std::string& name) :
      instr(nullptr), tag(name), bb(nullptr), tp(EXECUTION_ACTION_TAG) {}

    ExecutionAction(llvm::BasicBlock* const bb_) :
      instr(nullptr), tag(""), bb(bb_), tp(EXECUTION_ACTION_BASIC_BLOCK) {}

    ExecutionAction(const ExecutionAction& other) :
      instr(other.instr), tag(other.tag), bb(other.bb), tp(other.tp) {}
    
    ExecutionActionType type() const { return tp; }
    
    bool isInstruction() const {
      return type() == EXECUTION_ACTION_INSTRUCTION;
    }

    bool isTag() const {
      return type() == EXECUTION_ACTION_TAG;
    }

    bool isBasicBlock() const {
      return type() == EXECUTION_ACTION_BASIC_BLOCK;
    }

    BasicBlock* getBasicBlock() const {
      assert(isBasicBlock());
      return bb;
    }
    
    std::string getName() const {
      assert(isTag());
      return tag;
    }

    void setInstruction(llvm::Instruction* newInstr) {
      instr = newInstr;
      tp = EXECUTION_ACTION_INSTRUCTION;
    }

    llvm::Instruction* getInstruction() const {
      assert(isInstruction());
      return instr;
    }
  };

  static inline
  std::ostream& operator<<(std::ostream& out, const ExecutionAction& action) {
    if (action.isInstruction()) {
      out << valueString(action.getInstruction());
    }  else if (action.isBasicBlock()) {
      out << valueString(action.getBasicBlock());      
    } else {
      assert(action.isTag());
      out << action.getName();
      
    }
    return out;
  }

  static inline
  bool operator==(const ExecutionAction& a, const ExecutionAction& b) {
    if (a.type() != b.type()) {
      return false;
    }

    if (a.isInstruction() && b.isInstruction()) {
      return a.getInstruction() == b.getInstruction();
    }

    if (a.isBasicBlock() && b.isBasicBlock()) {
      return a.getBasicBlock() == b.getBasicBlock();
    }

    assert(a.isTag() && b.isTag());
    
    return a.getName() == b.getName();
  }
  
  static inline
  bool operator<(const ExecutionAction& a, const ExecutionAction& b) {
    if (a.type() != b.type()) {
      return a.type() < b.type();
    }

    if (a.isInstruction() && b.isInstruction()) {
      return a.getInstruction() < b.getInstruction();
    }

    if (a.isBasicBlock() && b.isBasicBlock()) {
      return a.getBasicBlock() < b.getBasicBlock();
    }

    assert(a.isTag() && b.isTag());
    
    return a.getName() < b.getName();
    
  }
  
  class Port {
  public:
    // TODO: registered is currently ignored in code generation. Registered
    // ports have separate companion reg variables inside generated verilog.
    // maybe I should print out output reg and remove internal regs?
    bool registered;    
    bool isInput;
    int width;
    std::string name;
    bool isDebug;

    bool input() const {
      return isInput;
    }

    bool output() const {
      return !isInput;
    }

    std::string toString() {
      return std::string(isInput ? "input" : "output") + (registered ? " reg " : "") + " [" + std::to_string(width - 1) + ":0] " + name;
    }
  };

  Port inputPort(const int width, const std::string& name);
  Port outputPort(const int width, const std::string& name);
  Port outputRegPort(const int width, const std::string& name);  
  Port outputDebugPort(const int width, const std::string& name);

  class ModuleSpec {
  public:
    std::map<std::string, std::string> params;
    std::string name;
    std::map<std::string, Port> ports;
    std::map<std::string, int> defaultValues;
  };

  class MemorySpec {
  public:
    int readLatency;
    int writeLatency;
    int numReadPorts;
    int numWritePorts;
    int width;
    int depth;
    bool addressable;
    ModuleSpec modSpec;
  };

  static inline MemorySpec registerSpec(const int width) {
    return {0, 1, 1, 1, width, 1, false, {{{"width", std::to_string(width)}}, "register"}};
  }

  static inline MemorySpec ramSpec(const int readLat,
                                   const int writeLat,
                                   const int nReadPorts,
                                   const int nWritePorts,
                                   const int width,
                                   const int depth) {
    return {readLat, writeLat, nReadPorts, nWritePorts, width, depth, true, {{{"width", std::to_string(32)}}, "RAM"}};
  }
  
  enum OperationType {
    RETURN_OP,
    PHI_OP,
    STORE_OP,
    LOAD_OP,
    ADD_OP,
    FADD_OP,    
    SUB_OP,
    MUL_OP,
    DIV_OP,
    SDIV_OP,
    CMP_OP,
    BR_OP,
    ZEXT_OP,
    SELECT_OP,
    NO_OP,
    SEXT_OP,
    CALL_OP,
  };

  static inline std::vector<OperationType> allOps() {
    return {
      RETURN_OP,
        PHI_OP,
        STORE_OP,
        LOAD_OP,
        ADD_OP,
        FADD_OP,        
        SUB_OP,
        MUL_OP,
        DIV_OP,
        SDIV_OP,
        CMP_OP,
        BR_OP,
        ZEXT_OP,
        SELECT_OP,
        NO_OP,
        SEXT_OP,
        CALL_OP
        };

  }

  OperationType opType(llvm::Instruction* const iptr);

  // enum FifoInterface {
  //   FIFO_TIMED,
  //   FIFO_RV,
  // };

  // class FifoSpec {
  // public:
  //   int readDelay;
  //   int writeDelay;
  //   FifoInterface interface;

  //   FifoSpec() : readDelay(0), writeDelay(0), interface(FIFO_RV) {}
    
  //   FifoSpec(const int rd_, const int wd_, const FifoInterface interface_) :
  //     readDelay(rd_), writeDelay(wd_), interface(interface_) {}
  // };
  
  class HardwareConstraints {

    std::map<OperationType, int> latencies;
    std::map<OperationType, int> counts;
    
  public:

    std::map<llvm::Value*, MemorySpec> memSpecs;
    std::map<llvm::Instruction*, llvm::Value*> memoryMapping;
    std::map<llvm::Value*, ModuleSpec> modSpecs;

    int getLatency(const OperationType op) const {
      return dbhc::map_find(op, latencies);
    }

    int getLatency(llvm::Instruction* iptr) const;

    bool isLimitedResource(const OperationType op) const {
      return dbhc::contains_key(op, counts);
    }

    int getCount(const OperationType op) const {
      // If not explicitly constrained we have an infinite number
      if (!dbhc::contains_key(op, counts)) {
        return 100000000;
      }

      return dbhc::map_find(op, counts);
    }
    
    void setLatency(const OperationType op, const int latency) {
      latencies[op] = latency;
    }

    void setCount(const OperationType op, const int count) {
      counts[op] = count;
    }
    
  };

  static inline
  void setMemSpec(const std::string& ramName,
                  HardwareConstraints& hcs,
                  llvm::Function* f,
                  MemorySpec spec) {
    bool found = false;
    for (auto& bb : f->getBasicBlockList()) {
      for (auto& instr : bb) {
        if (llvm::AllocaInst::classof(&instr)) {
          if (instr.getName() == ramName) {
            hcs.memSpecs[llvm::dyn_cast<llvm::Value>(&instr)] = spec;
            found = true;
          }
        }
      }
    }

    assert(found);
  }

  static inline
  void setMemSpec(llvm::Value* const val,
                  HardwareConstraints& hcs,
                  MemorySpec spec) {
    hcs.memSpecs[val] = spec;
  }
  
  static inline
  void setAllAllocaMemTypes(HardwareConstraints& hcs,
                            llvm::Function* f,
                            MemorySpec spec) {
    //bool found = false;
    for (auto& bb : f->getBasicBlockList()) {
      for (auto& instr : bb) {
        if (llvm::AllocaInst::classof(&instr)) {
          hcs.memSpecs[llvm::dyn_cast<llvm::Value>(&instr)] = spec;
          //found = true;
        }
      }
    }

    //assert(found);
  }
  
  static inline
  void addMemInfo(HardwareConstraints& hcs,
                  const std::map<llvm::Value*, MemorySpec>& mems) {
    for (auto m : mems) {
      assert(!dbhc::contains_key(m.first, hcs.memSpecs));
      hcs.memSpecs[m.first] = m.second;
      assert(dbhc::contains_key(m.first, hcs.memSpecs));
    }
  }

  class Schedule {

  public:
    std::map<llvm::Instruction*, std::vector<int> > instrTimes;
    std::map<llvm::BasicBlock*, std::vector<int> > blockTimes;
    std::map<llvm::BasicBlock*, int> pipelineSchedules;

    int startTime(llvm::Instruction* const instr) const {
      return dbhc::map_find(instr, instrTimes).front();
    }

    int clockTicksToFinish() const {
      int maxFinishTime = 0;
      for (auto b : blockTimes) {
        if (b.second.back() > maxFinishTime) {
          maxFinishTime = b.second.back();
        }
      }
      return maxFinishTime;
    }

    int numStates() const {
      return clockTicksToFinish() + 1;
    }

    void print(std::ostream& out) const {
      out << "Block times" << std::endl;
      for (auto i : blockTimes) {
        out << "\t" << i.first << " -> [";
        out << commaListString(i.second);
        out << "]" << std::endl;
      }

      out << "Instruction times" << std::endl;
      for (auto i : instrTimes) {
        out << "\t" << instructionString(i.first) << " -> [";
        out << commaListString(i.second);
        out << "]" << std::endl;
      }


    }

  };

  static inline std::ostream& operator<<(std::ostream& out, const Schedule& s) {
    s.print(out);
    return out;
  }
  
  Schedule scheduleFunction(llvm::Function* f, HardwareConstraints& hdc);
  Schedule scheduleFunction(llvm::Function* f,
                            HardwareConstraints& hdc,
                            std::set<llvm::BasicBlock*>& toPipeline);
  
  Schedule schedulePipeline(llvm::BasicBlock* const bb, HardwareConstraints& hdc);

  // Logical condition used in state transitions

  class Atom {
  public:

    llvm::Value* cond;
    bool negated;
    Atom() : cond(nullptr), negated(false) {}
    Atom(llvm::Value* const cond_) : cond(cond_), negated(false) {}
    Atom(llvm::Value* const cond_, const bool negated_) :
      cond(cond_), negated(negated_) {}    
  };

  static inline std::ostream& operator<<(std::ostream& out, const Atom& c) {
    if (c.cond != nullptr) {

      if (c.negated) {
        out << "!(";
      }

      if (llvm::Instruction::classof(c.cond)) {
        out << instructionString(static_cast<llvm::Instruction* const>(c.cond));
      } else {
        out << valueString(c.cond);
      }

      if (c.negated) {
        out << ")";
      }

    } else {
      out << "True";
    }
    return out;
  }
  
  class Condition {
  public:

    std::vector<std::vector<Atom> > clauses;
    // llvm::Value* cond;
    // bool negated;
    Condition() : clauses({{}}) {}

    Condition(llvm::Value* const cond_) : clauses({{{cond_, false}}}) {}
    Condition(llvm::Value* const cond_, const bool negated_) :
      clauses({{{cond_, negated_}}}) {}      

    Condition(std::vector<std::vector<Atom > > cl) :
      clauses(cl) {}

    bool isTrue() const {
      return (clauses.size() == 1) && (clauses[0].size() == 0);
    }

  };

  static inline std::ostream& operator<<(std::ostream& out, const Condition& c) {
    if ((c.clauses.size() == 1) && (c.clauses[0].size() == 0)) {
      out << "True" << std::endl;
      return out;
    }

    int nCl = 0;
    for (auto cl : c.clauses) {

      int nAt = 0;

      out << "(";
      for (auto atom : cl) {
        out << atom;
        if (nAt < (int) (cl.size() - 1)) {
          out << " ^ ";
        }
        nAt++;
      }
      out << ")";

      if (nCl < (int) (c.clauses.size() - 1)) {
        out << " v ";
      }
      nCl++;
    }

    return out;
  }

  class GuardedInstruction {
  public:
    llvm::Instruction* instruction;
    //Condition cond;
  };

  std::ostream& operator<<(std::ostream& out, const GuardedInstruction& t);

  typedef int StateId;

  class StateTransition {
  public:
    StateId dest;
    Condition cond;
  };

  static inline
  std::ostream&
  operator<<(std::ostream& out, const StateTransition& t) {
    out << t.dest << " if " << t.cond << std::endl;
    return out;
  }

  class Pipeline {
    int ii;
    int stateDepth;
    std::vector<StateId> states;    

  public:
    Pipeline(const int ii_,
             const int stateDepth_,
             const std::vector<StateId>& states_) :
      ii(ii_), stateDepth(stateDepth_), states(states_) {
      assert(II() >= 1);
      assert(depth() >= 1);
      assert(depth() == (int) states.size());
    }

    int II() const { return ii; }
    int depth() const { return stateDepth; }
    const std::vector<StateId>& getStates() const { return states; }
  };
  
  class StateTransitionGraph {
  public:

    Schedule sched;
    std::map<StateId, std::vector<GuardedInstruction> > opStates;
    std::map<StateId, std::vector<StateTransition> > opTransitions;
    std::vector<Pipeline> pipelines;

    StateTransitionGraph() {}

    llvm::Function* getFunction() const {
      auto& opSt = *(std::begin(opStates));
      return (opSt.second)[0].instruction->getParent()->getParent();
    }
    
    StateTransitionGraph(const StateTransitionGraph& other) {
      std::cout << "Calling stg const ref constructor" << std::endl;
      sched = other.sched;
      opStates = other.opStates;
      opTransitions = other.opTransitions;
      pipelines = other.pipelines;

      assert(sched.numStates() == other.sched.numStates());
      assert(opStates.size() == other.opStates.size());
      assert(opTransitions.size() == other.opTransitions.size());
      assert(pipelines.size() == other.pipelines.size());      
    }

    StateTransitionGraph(Schedule& sched_) : sched(sched_) {}

    StateId instructionStartState(llvm::Instruction* const instr) {
      return dbhc::map_find(instr, sched.instrTimes).front();
    }

    StateId instructionEndState(llvm::Instruction* const instr) {
      return dbhc::map_find(instr, sched.instrTimes).back();
    }
    
    std::vector<GuardedInstruction>
    instructionsFinishingAt(const StateId id) const {
      std::vector<GuardedInstruction> instrs;
      for (auto st : dbhc::map_find(id, opStates)) {
        llvm::Instruction* instr = st.instruction;
        if (id == dbhc::map_find(instr, sched.instrTimes).back()) {
          instrs.push_back(st);
        }
      }
      return instrs;
    }

    std::vector<GuardedInstruction>
    instructionsStartingAt(const StateId id) const {
      std::vector<GuardedInstruction> instrs;
      for (auto st : dbhc::map_find(id, opStates)) {
        llvm::Instruction* instr = st.instruction;
        if (id == dbhc::map_find(instr, sched.instrTimes).front()) {
          instrs.push_back(st);
        }
      }
      return instrs;
    }
    
    int numControlStates() const {
      return opStates.size();
    }

    bool hasTransition(const StateId a, const StateId b) const {
      if (!dbhc::contains_key(a, opTransitions)) {
        return false;
      }

      auto nextStates = dbhc::map_find(a, opTransitions);
      for (auto t : nextStates) {
        if (t.dest == b) {
          return true;
        }
      }

      return false;
    }
    void print(std::ostream& out) {
      out << "--- States" << std::endl;
      for (auto st : opStates) {
        out << "\t" << st.first << std::endl;
        for (auto instr : st.second) {
          out << instr << std::endl;
        }
      }

      out << "--- State Transistions" << std::endl;      
      for (auto tr : opTransitions) {
        out << "\t" << tr.first << std::endl;
        for (auto nextState : tr.second) {
          out << "\t\t -> " << nextState << std::endl;
        }
      }
    }

  };

  typedef StateTransitionGraph STG;

  STG buildSTG(Schedule& sched, llvm::Function* const f);

  STG buildSTG(Schedule& sched,
               llvm::Function* const f,
               std::function<void(Schedule&,
                                  STG&,
                                  StateId,
                                  llvm::ReturnInst*,
                                  Condition&)>& returnBehavior);
  
  STG buildSTG(Schedule& sched,
               llvm::BasicBlock* entryBlock,
               std::set<llvm::BasicBlock*>& blockList,
               std::function<void(Schedule&,
                                  STG&,
                                  StateId,
                                  llvm::ReturnInst*,
                                  Condition&)>& returnBehavior);
  
  HardwareConstraints standardConstraints();

  class LinearExpression {
    std::map<std::string, int> vars;
    int c;

  public:

    LinearExpression(const std::string var_) :
      vars({{var_, 1}}), c(0) {}

    LinearExpression(int c_) :
      vars({}), c(c_) {}
    
    LinearExpression(std::map<std::string, int>& vars_, const int c_) :
      vars(vars_), c(c_) {}

    LinearExpression() : vars({}), c(0) {}

    LinearExpression scalarMul(const int k) const {
      std::map<std::string, int> mulVars;
      for (auto v : vars) {
        mulVars.insert({v.first, k*v.second});
      }
      return {mulVars, k*getCoeff()};
    }
    
    LinearExpression sub(const LinearExpression& right) const {
      std::map<std::string, int> subVars;
      auto rightVars = right.getVars();
      for (auto v : vars) {
        if (dbhc::contains_key(v.first, rightVars)) {
          subVars.insert({v.first, v.second - dbhc::map_find(v.first, rightVars)});
        } else {
          subVars.insert(v);
        }
      }

      for (auto v : rightVars) {
        if (!dbhc::contains_key(v.first, vars)) {
          subVars.insert({v.first, -v.second});
        }
      }

      return {subVars, c - right.getCoeff()};
    }

    LinearExpression add(const LinearExpression& right) const {
      std::map<std::string, int> addVars;
      auto rightVars = right.getVars();
      for (auto v : vars) {
        if (dbhc::contains_key(v.first, rightVars)) {
          addVars.insert({v.first, v.second + dbhc::map_find(v.first, rightVars)});
        } else {
          addVars.insert(v);
        }
      }

      for (auto v : rightVars) {
        if (!dbhc::contains_key(v.first, vars)) {
          addVars.insert({v.first, v.second});
        }
      }

      return {addVars, c + right.getCoeff()};
    }
    
    int getCoeff() const {
      return c;
    }

    std::map<std::string, int> getVars() const {
      return vars;
    }

    int getValue(const std::string& var) {
      assert(dbhc::contains_key(var, vars));
      return dbhc::map_find(var, vars);
    }
  };

  static inline
  std::ostream& operator<<(std::ostream& out, const LinearExpression expr) {
    auto vars = expr.getVars();
    for (auto v : vars) {
      out << v.second << "*" << v.first << " + ";
    }
    out << expr.getCoeff();

    return out;
  }

  enum ZCondition {
    CMP_LTZ,
    CMP_GTZ,
    CMP_LTEZ,
    CMP_GTEZ,
    CMP_EQZ
  };

  class LinearConstraint {
  public:
    LinearExpression expr;
    ZCondition cond;
  };

  static inline
  std::string toString(const ZCondition cond) {
    switch (cond) {
    case CMP_LTZ:
      return "< 0";
    case CMP_GTEZ:
      return ">= 0";
    case CMP_LTEZ:
      return "<= 0";
    case CMP_GTZ:
      return "> 0";
    case CMP_EQZ:
      return "== 0";
      
    default:
      assert(false);
    }
  }

  static inline
  std::ostream& operator<<(std::ostream& out, const LinearConstraint& c) {
    out << c.expr << " " << toString(c.cond);
    return out;
  }

  class SchedulingProblem {
  public:
    int blockNo;

    std::map<ExecutionAction, std::vector<std::string> > actionVarNames;
    std::map<llvm::Instruction*, std::vector<std::string> > schedVarNames;
    std::map<llvm::BasicBlock*, std::vector<std::string> > blockVarNames;
    std::map<llvm::BasicBlock*, std::string> IInames;

    std::vector<LinearConstraint> constraints;

    HardwareConstraints hdc;
    bool optimize;
    LinearExpression objectiveFunction;

    SchedulingProblem(const HardwareConstraints& hcs_) :
      blockNo(0), hdc(hcs_), optimize(false) {}

    SchedulingProblem() : optimize(false) {
      blockNo = 0;
    }

    void setObjective(const LinearExpression& expr) {
      objectiveFunction = expr;
      optimize = true;
    }

    std::string getIIName(llvm::BasicBlock* bb) const {
      std::string val = dbhc::map_find(bb, IInames);
      return val;
    }
    
    LinearExpression getII(llvm::BasicBlock* bb) const {
      std::string val = getIIName(bb);
      return LinearExpression(getIIName(bb));
    }
    
    int blockNumber() const {
      return blockNo;
    }

    LinearExpression blockStart(llvm::BasicBlock* bb) {
      return LinearExpression(dbhc::map_find(bb, blockVarNames).front());
    }

    LinearExpression blockEnd(llvm::BasicBlock* bb) {
      return LinearExpression(dbhc::map_find(bb, blockVarNames).back());
    }

    bool hasAction(const ExecutionAction& action) {
      return dbhc::contains_key(action, actionVarNames);
    }

    void addAction(const ExecutionAction& action) {
      // TODO: Eventually use this for all actions including instructions
      assert(!action.isInstruction());
      actionVarNames[action] = {action.getName() + "_start", action.getName() + "_end"};
    }

    LinearExpression actionStart(const ExecutionAction& action) {
      auto lc = LinearExpression(dbhc::map_find(action, actionVarNames).front());
      cout << "start of " << action << " is " << lc << endl;
      return lc;
    }

    LinearExpression actionEnd(const ExecutionAction& action) {
      return LinearExpression(dbhc::map_find(action, actionVarNames).back());
    }
    
    LinearExpression instrStart(const ExecutionAction& action) {
      assert(action.isInstruction());
      return LinearExpression(dbhc::map_find(action.getInstruction(), schedVarNames).front());
    }

    LinearExpression instrEnd(const ExecutionAction& action) {
      assert(action.isInstruction());      
      return LinearExpression(dbhc::map_find(action.getInstruction(), schedVarNames).back());
    }
    
    LinearExpression instrStart(llvm::Instruction* instr) {
      return LinearExpression(dbhc::map_find(instr, schedVarNames).front());
    }

    LinearExpression instrStage(llvm::Instruction* instr, const int i) {
      return LinearExpression(dbhc::map_find(instr, schedVarNames).at(i));
    }

    LinearExpression instrEnd(llvm::Instruction* instr) {
      return LinearExpression(dbhc::map_find(instr, schedVarNames).back());
    }

    int numStages(llvm::Instruction* instr) {
      return (int) dbhc::map_find(instr, schedVarNames).size();
    }
    
    void addBasicBlock(llvm::BasicBlock* const bb);

    void addConstraint(const LinearConstraint& constraint) {
      constraints.push_back(constraint);
    }
  };

  static inline
  LinearExpression
  operator-(const LinearExpression left, const LinearExpression right) {
    return left.sub(right);
  }

  static inline
  LinearExpression
  operator+(const LinearExpression left, const LinearExpression right) {
    return left.add(right);
  }

  static inline
  LinearExpression
  operator*(const LinearExpression left, const int c) {
    return left.scalarMul(c);
  }
  
  static inline
  LinearConstraint
  operator>=(const LinearExpression left, const LinearExpression right) {
    return {left - right, CMP_GTEZ};
  }

  static inline
  LinearConstraint
  operator>=(const LinearExpression left, const int right) {
    return {left - LinearExpression(right), CMP_GTEZ};
  }

  static inline
  LinearConstraint
  operator>=(const int left, const LinearExpression right) {
    return {LinearExpression(left) - right, CMP_GTEZ};
  }

  static inline
  LinearConstraint
  operator<=(const LinearExpression left, const LinearExpression right) {
    return {left - right, CMP_LTEZ};
  }

  static inline
  LinearConstraint
  operator<=(const LinearExpression left, const int right) {
    return {left - LinearExpression(right), CMP_LTEZ};
  }

  static inline
  LinearConstraint
  operator<=(const int left, const LinearExpression right) {
    return {LinearExpression(left) - right, CMP_LTEZ};
  }

  static inline
  LinearConstraint
  operator==(const LinearExpression left, const LinearExpression right) {
    return {left - right, CMP_EQZ};
  }

  static inline
  LinearConstraint
  operator==(const LinearExpression left, const int right) {
    return {left - LinearExpression(right), CMP_EQZ};
  }

  static inline
  LinearConstraint
  operator==(const int left, const LinearExpression right) {
    return {LinearExpression(left) - right, CMP_EQZ};
  }

  // ---
  static inline
  LinearConstraint
  operator<(const LinearExpression left, const LinearExpression right) {
    return {left - right, CMP_LTZ};
  }

  static inline
  LinearConstraint
  operator<(const LinearExpression left, const int right) {
    return {left - LinearExpression(right), CMP_LTZ};
  }

  static inline
  LinearConstraint
  operator>(const int left, const LinearExpression right) {
    return {LinearExpression(left) - right, CMP_GTZ};
  }

  static inline
  LinearConstraint
  operator>(const LinearExpression left, const LinearExpression right) {
    return {left - right, CMP_GTZ};
  }

  static inline
  LinearConstraint
  operator>(const LinearExpression left, const int right) {
    return {left - LinearExpression(right), CMP_GTZ};
  }

  static inline
  LinearConstraint
  operator<(const int left, const LinearExpression right) {
    return {LinearExpression(left) - right, CMP_LTZ};
  }
  
  Schedule buildFromModel(SchedulingProblem& p);

  // TODO: Add incremental support for building scheduling constraints?
  Schedule scheduleFunction(llvm::Function* f,
                            HardwareConstraints& hdc,
                            std::set<llvm::BasicBlock*>& toPipeline,
                            std::map<llvm::Function*, SchedulingProblem>& constraints);

  SchedulingProblem
  createSchedulingProblem(llvm::Function* f,
                          HardwareConstraints& hdc,
                          std::set<llvm::BasicBlock*>& toPipeline);

  class ExecutionEvent {
  public:
    ExecutionAction action;
    bool isEnd;
  };

  static inline
  bool operator<(const ExecutionEvent a, const ExecutionEvent b) {
    if (a.action == b.action) {
      return a.isEnd < b.isEnd;
    }

    return a.action < b.action;
  }

  static inline
  std::ostream& operator<<(std::ostream& out, const ExecutionEvent a) {
    std::string prefix = a.isEnd ? "end" : "start";
    out << prefix << "(" << a.action << ")";
    return out;
  }
  
  class EventTime {
  public:
    // TODO: Replace with ExecutionEvent
    ExecutionAction action;
    bool isEnd;
    int offset;

    EventTime() : action(), isEnd(false), offset(0) {}
    
    EventTime(const EventTime& other) : action(other.action), isEnd(other.isEnd), offset(other.offset) {}

    EventTime(const ExecutionAction& action_,
              bool isEnd_,
              int offset_) : action(action_), isEnd(isEnd_), offset(offset_) {}    

    ExecutionEvent event() const {
      return {action, isEnd};
    }

    void print(std::ostream& out) const {
      std::string pre = isEnd ? "end" : "start";
      out << pre << "(" << action << ") + " << offset;
    }
    
    void replaceInstruction(Instruction* const toReplace,
                            Instruction* const replacement) {
      if (action.isInstruction() && (action.getInstruction() == toReplace)) {
        action.setInstruction(replacement);
        //instr = replacement;
      }
    }

    void replaceAction(ExecutionAction& toReplace,
                       ExecutionAction& replacement) {
      if (action == toReplace) {
        action = replacement;
      }
    }
    
    bool isStart() const {
      return !isEnd;
    }

    ExecutionAction getAction() const {
      return action;
    }
    
    llvm::Instruction* getInstr() const {
      return action.getInstruction();
    }
  };

  typedef EventTime InstructionTime;

  static inline   
  InstructionTime operator+(const InstructionTime t, const int offset) {
    return {t.action, t.isEnd, t.offset + offset};
  }

  static inline   
  InstructionTime operator+(const int offset, const InstructionTime t) {
    return {t.action, t.isEnd, t.offset + offset};
  }

  static inline   
  LinearExpression
  toLinearExpression(const InstructionTime& time,
                     SchedulingProblem& p) {
    if (time.action.isInstruction()) {
      return (time.isEnd ? p.instrEnd(time.action) : p.instrStart(time.action)) + time.offset;
    } else if (time.action.isBasicBlock()) {
      return (time.isEnd ? p.blockEnd(time.action.getBasicBlock()) : p.blockStart(time.action.getBasicBlock())) + time.offset;
    } else {
      if (!p.hasAction(time.action)) {
        p.addAction(time.action);
      }
      return (time.isEnd ? p.actionEnd(time.action) : p.actionStart(time.action)) + time.offset;      
    }
  }

  static inline   
  InstructionTime start(BasicBlock* const bb) {
    return {bb, false, 0};
  }

  static inline   
  InstructionTime end(BasicBlock* const bb) {
    return {bb, true, 0};
  }
  
  static inline   
  InstructionTime instrEnd(Instruction* const instr) {
    return {instr, true, 0};
  }

  static inline   
  InstructionTime instrStart(Instruction* const instr) {
    return {instr, false, 0};
  }

  static inline   
  InstructionTime actionStart(ExecutionAction& action) {
    return {action, false, 0};
  }

  static inline   
  InstructionTime actionEnd(ExecutionAction& action) {
    return {action, true, 0};
  }
  
  static inline   
  InstructionTime instrEnd(llvm::Value* const instr) {
    assert(llvm::Instruction::classof(instr));
    return {llvm::dyn_cast<llvm::Instruction>(instr), true, 0};
  }

  static inline 
  InstructionTime instrStart(llvm::Value* const instr) {
    assert(llvm::Instruction::classof(instr));
    return {llvm::dyn_cast<llvm::Instruction>(instr), false, 0};
  }

  static inline
  std::ostream& operator<<(std::ostream& out, const InstructionTime& t) {
    t.print(out);
    return out;
  }

  enum ExecutionConstraintType {
    CONSTRAINT_TYPE_ORDERED,
    CONSTRAINT_TYPE_STALL,    
  };

  class ExecutionConstraint {
  public:
    virtual void addSelfTo(SchedulingProblem& p, Function* f) = 0;
    virtual ExecutionConstraintType type() const = 0;
    virtual void replaceInstruction(Instruction* const toReplace,
                                    Instruction* const replacement) = 0;

    virtual void replaceAction(ExecutionAction& toReplace,
                               ExecutionAction& replacement) = 0;
    
    virtual void replaceStart(Instruction* const toReplace,
                              Instruction* const replacement) = 0;
    virtual void replaceEnd(Instruction* const toReplace,
                            Instruction* const replacement) = 0;

    virtual bool references(Instruction* instr) const = 0;
    virtual ExecutionConstraint* clone() const = 0;
    virtual void print(std::ostream& out) const = 0;    
    virtual ~ExecutionConstraint() {}
  };

  static inline
  std::ostream& operator<<(std::ostream& out, const ExecutionConstraint& c) {
    c.print(out);
    return out;
  }

  class WaitUntil : public ExecutionConstraint {
  public:
    Value* value;
    Instruction* mustWait;
  };

  enum OrderRestriction {
    ORDER_RESTRICTION_SIMULTANEOUS,
    ORDER_RESTRICTION_BEFORE,
    ORDER_RESTRICTION_BEFORE_OR_SIMULTANEOUS,
  };

  static inline
  std::string toString(OrderRestriction r) {
    switch(r) {
    case ORDER_RESTRICTION_SIMULTANEOUS:
      return "==";
    case ORDER_RESTRICTION_BEFORE:
      return "<";
    case ORDER_RESTRICTION_BEFORE_OR_SIMULTANEOUS:
      return "<=";
    default:
      assert(false);
    }
  }

  class Ordered : public ExecutionConstraint {
  public:
    InstructionTime before;
    InstructionTime after;

    OrderRestriction restriction;

    Ordered(const InstructionTime before_,
            const InstructionTime after_,
            const OrderRestriction restriction_) :
      before(before_),
      after(after_),
      restriction(restriction_) {}

    virtual void replaceStart(Instruction* const toReplace,
                              Instruction* const replacement) override {
      if (before.isStart()) {
        before.replaceInstruction(toReplace, replacement);
      }
      if (after.isStart()) {
        after.replaceInstruction(toReplace, replacement);
      }
    }

    virtual void replaceEnd(Instruction* const toReplace,
                            Instruction* const replacement) override {
      if (before.isEnd) {
        before.replaceInstruction(toReplace, replacement);
      }

      if (after.isEnd) {
        after.replaceInstruction(toReplace, replacement);
      }
      
    }
    
    virtual ExecutionConstraintType type() const override {
      return CONSTRAINT_TYPE_ORDERED;
    }

    virtual void print(std::ostream& out) const override {
      before.print(out);
      out << " " << toString(restriction) << " ";
      after.print(out);
    }
    
    virtual ExecutionConstraint* clone() const override {
      InstructionTime beforeCpy(before);
      InstructionTime afterCpy(after);      
      return new Ordered(beforeCpy, afterCpy, restriction);
    }
    
    virtual bool references(Instruction* instr) const override {
      if (before.action.isInstruction() && (before.action.getInstruction() == instr)) {
        return true;
      }

      if (after.action.isInstruction() && (after.action.getInstruction() == instr)) {
        return true;
      }

      return false;
    }

    virtual void replaceAction(ExecutionAction& toReplace,
                               ExecutionAction& replacement) override {
      before.replaceAction(toReplace, replacement);
      after.replaceAction(toReplace, replacement);      
    }
    
    virtual void replaceInstruction(Instruction* const toReplace,
                                    Instruction* const replacement) override {
      before.replaceInstruction(toReplace, replacement);
      after.replaceInstruction(toReplace, replacement);      
    }
    
    virtual void addSelfTo(SchedulingProblem& p, Function* f) override {
      LinearExpression aTime = toLinearExpression(after, p);
      LinearExpression bTime = toLinearExpression(before, p);
      if (restriction == ORDER_RESTRICTION_SIMULTANEOUS) {
        p.addConstraint(bTime == aTime);
      } else if (restriction == ORDER_RESTRICTION_BEFORE) {
        p.addConstraint(bTime < aTime);
      } else if (restriction == ORDER_RESTRICTION_BEFORE_OR_SIMULTANEOUS) {
        p.addConstraint(bTime <= aTime);        
      } else {
        assert(false);
      }
    }
  };

  static inline
  Ordered* operator<(InstructionTime before, InstructionTime after) {
    return new Ordered(before, after, ORDER_RESTRICTION_BEFORE);
  }

  static inline
  Ordered* operator==(InstructionTime before, InstructionTime after) {
    return new Ordered(before, after, ORDER_RESTRICTION_SIMULTANEOUS);
  }

  static inline
  Ordered* operator<=(InstructionTime before, InstructionTime after) {
    return new Ordered(before, after, ORDER_RESTRICTION_BEFORE_OR_SIMULTANEOUS);
  }

  static inline
  Ordered* operator>(InstructionTime before, InstructionTime after) {
    return after < before;
  }
  
  class ExecutionConstraints {
  public:
    std::vector<ExecutionConstraint*> constraints;

    std::vector<ExecutionConstraint*>
    //    constraintsOnStart(llvm::Instruction* const instr) const {
    constraintsOnStart(ExecutionAction instr) const {    
      std::vector<ExecutionConstraint*> on;      
      for (auto c : constraints) {
        if (c->type() == CONSTRAINT_TYPE_ORDERED) {
          Ordered* oc = static_cast<Ordered*>(c);

          if (oc->after.isStart() &&
              (oc->after.action == instr)) {
            on.push_back(c);
          }

          // else if (oc->before.isStart() &&
          //            (oc->before.action == instr)) {
          //   on.push_back(c);
          // }

        } else {
          std::cout << "No constraint on stalls yet" << std::endl;
          assert(false);
        }
      }
      return on;
    }

    void remove(ExecutionConstraint* c) {
      assert(dbhc::elem(c, constraints));

      dbhc::remove(c, constraints);
      delete c;
    }

    std::vector<ExecutionConstraint*>
    constraintsOnEnd(ExecutionAction instr) const {
      std::vector<ExecutionConstraint*> on;
      for (auto c : constraints) {
        if (c->type() == CONSTRAINT_TYPE_ORDERED) {
          Ordered* oc = static_cast<Ordered*>(c);

          if (oc->after.isEnd &&
              (oc->after.action == instr)) {

            on.push_back(c);
          }
        } else {
          std::cout << "No constraint on stalls yet" << std::endl;
          assert(false);
        }
      }
      return on;
    }
    
    void addConstraint(ExecutionConstraint* c) {
      constraints.push_back(c);
    }

    void add(ExecutionConstraint* c) {
      addConstraint(c);
    }
    
    void addConstraints(SchedulingProblem& p,
                        Function* f) {
      for (auto c : constraints) {
        //std::cout << "Adding constraint " << *c << " to problem" << std::endl;
        c->addSelfTo(p, f);
      }
    }

    void startsBeforeStarts(Instruction* const before,
                            Instruction* const after) {
      constraints.push_back(new Ordered(instrStart(before), instrStart(after), ORDER_RESTRICTION_BEFORE));
    }
    
    void endsBeforeStarts(Instruction* const before,
                          Instruction* const after) {
      constraints.push_back(new Ordered(instrEnd(before), instrStart(after), ORDER_RESTRICTION_BEFORE));
    }

    void startsBeforeEnds(Instruction* const before,
                          Instruction* const after) {
      constraints.push_back(new Ordered(instrStart(before), instrEnd(after), ORDER_RESTRICTION_BEFORE));
    }
    
    void startSameTime(Instruction* const before,
                       Instruction* const after) {
      constraints.push_back(new Ordered(instrStart(before), instrStart(after), ORDER_RESTRICTION_SIMULTANEOUS));
    }
    
    ~ExecutionConstraints() {
      for (auto c : constraints) {
        delete c;
      }
    }
  };

  static inline
  void addDataConstraints(llvm::Function* f, ExecutionConstraints& exe) {
    for (auto& bb : f->getBasicBlockList()) {

      Instruction* term = bb.getTerminator();
      
      for (auto& instr : bb) {
        Instruction* iptr = &instr;
        
        for (auto& user : iptr->uses()) {
          assert(Instruction::classof(user));
          auto userInstr = dyn_cast<Instruction>(user.getUser());          

          if (!PHINode::classof(userInstr)) {
            // Instructions must finish before their dependencies
            exe.addConstraint(instrEnd(iptr) <= instrStart(userInstr));
          }
        }

        // Instructions must finish before their basic block terminator,
        // this is not true anymore in pipelining
        if (iptr != term) {
          exe.addConstraint(instrEnd(iptr) <= instrStart(term));
        }
      }
    }

  }


  class InterfaceFunctions {
  public:
    std::map<llvm::Function*, ExecutionConstraints> constraints;
    std::map<std::string,
             std::function<void(llvm::Function*, ExecutionConstraints&)> >
    functionTemplates;

    void addFunction(Function* const f) {
      constraints[f] = ExecutionConstraints();
    }

    ExecutionConstraints& getConstraints(llvm::Function* const f) {
      assert(dbhc::contains_key(f, constraints));
      return constraints.find(f)->second;
    }

    bool containsFunction(llvm::Function* const f) const {
      return dbhc::contains_key(f, constraints);
    }
    
  };

  void implementRVFifoRead(llvm::Function* readFifo, ExecutionConstraints& exec);
  void implementRVFifoWrite(llvm::Function* writeFifo, ExecutionConstraints& exec);
  void implementRVFifoWriteTemplate(llvm::Function* writeFifo,
                                    ExecutionConstraints& exec);
  void implementRVFifoWriteRef(llvm::Function* writeFifo,
                               ExecutionConstraints& exec);

  void inlineWireCalls(llvm::Function* f,
                       ExecutionConstraints& exec,
                       InterfaceFunctions& interfaces);

  ModuleSpec fifoSpec(int width, int depth);

  void inlineFunctionWithConstraints(llvm::Function* const f,
                                     ExecutionConstraints& exec,
                                     llvm::CallInst* const toInline,
                                     ExecutionConstraints& constraintsToInline);

  ModuleSpec wireSpec(int width);

  void implementWireRead(Function* readFifo);
  void implementWireWrite(Function* writeFifo);

  class InstanceConstraint {
  public:
    int offset;
    int instanceDifference;
    bool isStrict;
  };

  DirectedGraph<ExecutionEvent, InstanceConstraint>
  buildExeGraph(ExecutionConstraints& exec);

}
