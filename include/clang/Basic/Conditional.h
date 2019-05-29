//===-- clang/Basic/conditional.h - Instruction class definition -------*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the conditional class, the variable aware part of the
/// token
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_LEX_CONDITIONAL
#define LLVM_CLANG_LEX_CONDITIONAL
#define ifDefLiteralString "ifdef"

#include <string>
#include <vector>
#include <map>

namespace Variability {

enum PCType{
    TRUE = 0, AND, OR, NOT, LIT,
};

class PresenceCondition;
class And;
class Or;
class True;
class Not;
class Literal;

// This class is for holding the conditonal
class PresenceCondition {
    // For caching SAT solutions
    std::map<std::string, bool*> solve_map;
    // For indexing the bool array in solve_map
    // Example: solve_map(other->toString())[THIS_IMPLIES_OTHER] == true
    //   if "this implies other" is a tautology
    enum SolutionType {
        THIS_IMPLIES_OTHER = 0,
        THIS_IMPLIES_NOT_OTHER,
        OTHER_IMPLIES_THIS,
    };

public:
    PCType typeOfPC;
    const virtual std::string toString();
    bool isSatisfiable();
    virtual PresenceCondition* toCnf();
    virtual PresenceCondition* toNegationNormal();
    // This function returns the presence condition based on the stack given
    static PresenceCondition* getList(std::vector<bool> declarations,
        std::vector<std::string> names);

    // These three methods are called to determine if th parser should split
    bool ShouldSplitOnCondition(PresenceCondition* other);
    bool ShouldContinueOnCondition(PresenceCondition* other);
    bool ShouldSkipOnCondition(PresenceCondition* other);
    bool ShouldJoinOnCondition(PresenceCondition* other);
    // Returns true if this is logically equivalent to other
    bool EquivalentTo(PresenceCondition* other);
    // Returns true if this implies other is a tautology
    bool Implies(PresenceCondition* other);
    void solve(PresenceCondition* other);

    virtual ~PresenceCondition() {}
};

class True : public PresenceCondition {
    // Always true, tokens outside of any ifdef, and ifndef will have this
public:
    const std::string toString();
    PresenceCondition* toCnf();
    PresenceCondition* toNegationNormal();
    True(){
        this->typeOfPC = TRUE;
    }
};

class Literal : public PresenceCondition {
    // Literal presence condition ocurres when there is a single ifdef, also,
    // Not and And presence conditions are made up of these
    std::string literal;

public:
    const std::string toString();
    PresenceCondition* toCnf();
    PresenceCondition* toNegationNormal();

    Literal(std::string lit)
        : literal(lit)
    {
        this->typeOfPC = LIT;
    }
    ~Literal() {}
};

class And : public PresenceCondition {
    // When there is nested if[n]def's the and is needed to represent both
    // conditions and can be nested And(And(And(Literal("A"), Literal("B")),
    // Literal("C")), Literal("D")) as deep as needed.
    PresenceCondition* left;
    PresenceCondition* right;

public:
    const std::string toString();
    PresenceCondition* toCnf();
    PresenceCondition* toNegationNormal();
    And(PresenceCondition* left_, PresenceCondition* right_);
    ~And() {}

    friend Not;
    friend Or;
};

class Or : public PresenceCondition {
    // #if defined(A) || defined(B)
    PresenceCondition* left;
    PresenceCondition* right;

public:
    const std::string toString();
    PresenceCondition* toCnf();
    PresenceCondition* toNegationNormal();
    Or(PresenceCondition* left_, PresenceCondition* right_);
    ~Or() {}

    friend Not;
};

class Not : public PresenceCondition {
    // Not is needed when there are ifndef, or ifdef 0, then the not
    // of a literal is required
    PresenceCondition* right;

public:
    const std::string toString();
    PresenceCondition* toCnf();
    PresenceCondition* toNegationNormal();

    Not(PresenceCondition* right_) { this->right = right_; this->typeOfPC = NOT; }
    ~Not() {}
    friend Or;
    friend And;
};

} // namespace Variability

#endif
