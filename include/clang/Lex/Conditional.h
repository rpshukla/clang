//===-- clang/Lex/conditional.h - Instruction class definition -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the conditional class, the variable aware part of the token
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_LEX_CONDITIONAL
#define LLVM_CLANG_LEX_CONDITIONAL
#define ifDefLiteralString "ifdef"

#include <string>
#include <vector>

namespace Variablity{

    class PresenceCondition;
    class And;
    class True;
    class Not;
    class Literal;

    class PresenceCondition{
        // This class is for holding the conditonal
        public:
            virtual std::string toString();
            // This function returns the presence condition based on the stack given
            static PresenceCondition* getList(std::vector<bool> declarations, std::vector<std::string> names);

            virtual ~PresenceCondition() {}
    };

    class True: public PresenceCondition{
        // Always true, tokens outside of any ifdef, and ifndef will have this
        public:
            std::string toString();
    };

    class Literal: public PresenceCondition{
        // Literal presence condition ocurres when there is a single ifdef, also,
        // Not and And presence conditions are made up of these
        std::string literal;

        public:
        std::string toString();


        Literal(std::string lit): literal(lit){}
        ~Literal(){}
    };

    class And: public PresenceCondition {
        // When there is nested if[n]def's the and is needed to represent both conditions
        // and can be nested And(And(And(Literal("A"), Literal("B")), Literal("C")), Literal("D"))
        // as deep as needed.
        PresenceCondition* left;
        PresenceCondition* right;

        public:
        std::string toString();
        And(PresenceCondition *left_, PresenceCondition *right_);
        ~And(){}
    };


    class Not: public PresenceCondition {
        // Not is needed when there are ifndef, or ifdef 0, then the not
        // of a literal is required
        PresenceCondition* right;

        public:
        std::string toString();


        Not(PresenceCondition *right_){ this->right = right_; }
        ~Not(){}
    };

}


#endif
