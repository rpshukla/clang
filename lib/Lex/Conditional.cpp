#include "clang/Lex/Conditional.h"
using namespace Variablity;






std::string PresenceCondition::toString()
{
    // This should never be called, it should always be overridden
    return "##ERROR##";
}


std::string True::toString()
{
    return "True";
}

std::string Literal::toString()
{
    return this->literal;
}

std::string And::toString()
{
    return "(" + this->left->toString() + " && " + this->right->toString() + ")";
}

And::And(PresenceCondition *left_, PresenceCondition *right_)
{
    this->left = left_;
    this->right = right_;
}


std::string Not::toString()
{
    return "~" + this->right->toString();
}


PresenceCondition* PresenceCondition::getList(std::vector<bool> declarations, std::vector<std::string> names)
{
    // unpacks the vectors from the condition stack to form the current conditional
    if (names.size() == 1)
    {
        return new True();
    }
    else if (names.size() == 0)
    {
        return declarations[1]?
               static_cast<PresenceCondition*>(new Literal(names[1])) :
               static_cast<PresenceCondition*>(new Not(new Literal(names[1])));
    }
    else
    {
        PresenceCondition* pc = declarations[0] ?
                                static_cast<PresenceCondition*>(new Literal(names[0])) :
                                static_cast<PresenceCondition*>(new Not(new Literal(names[0])));

        for (unsigned long i = 1, e = names.size(); i < e; ++i)
        {
            pc = new And(pc, declarations[i] ?
                         static_cast<PresenceCondition*>(new Literal(names[i])) :
                         static_cast<PresenceCondition*>(new Not(new Literal(names[i]))));
        }
        return pc;
    }

}
