#include "clang/Lex/Conditional.h"
#include <ctype.h>
#include <cryptominisat5/cryptominisat.h>

using namespace Variablity;



bool PresenceCondition::ShouldSplitOnCondition(PresenceCondition* other) {
    if(this->solve_map.find(other->toString()) == this->solve_map.end()){
        this->solve(other);
    }
    return this->solve_map[other->toString()][0] && this->solve_map[other->toString()][1];
}
bool PresenceCondition::ShouldJoinOnCondition(PresenceCondition* other) {
    if(this->solve_map.find(other->toString()) == this->solve_map.end()){
        this->solve(other);
    }
    return this->solve_map[other->toString()][2] && !this->solve_map[other->toString()][3];
}
bool PresenceCondition::ShouldContinueOnCondition(PresenceCondition* other) {
    if(this->solve_map.find(other->toString()) == this->solve_map.end()){
        this->solve(other);
    }
    return this->solve_map[other->toString()][0] && !this->solve_map[other->toString()][1];
}
bool PresenceCondition::ShouldSkipOnCondition(PresenceCondition* other) {
    if(this->solve_map.find(other->toString()) == this->solve_map.end()){
        this->solve(other);
    }
    return !this->solve_map[other->toString()][0] && this->solve_map[other->toString()][1];
}

void PresenceCondition::solve(PresenceCondition* other){
    PresenceCondition* equ_1 = new And(this, (new Or(new Not(this), other)));
    PresenceCondition* equ_2 = new And(this, (new Or(new Not(this), new Not(other))));

    PresenceCondition* equ_3 = new And(other, new Not(this));
    PresenceCondition* equ_4 = new And(this, new Not(other));



    bool* ans = new bool[4];
    ans[0] = equ_1->isSatisfiable(); // parser implies token
    ans[1] = equ_2->isSatisfiable(); // parser implies not token
    ans[2] = equ_3->isSatisfiable(); // not (token implies parser)
    ans[3] = equ_4->isSatisfiable(); // not (parser implies token)
    this->solve_map[other->toString()] = ans;


    delete equ_1;
    delete equ_2;
    delete equ_3;
    delete equ_4;
}




using namespace CMSat;
bool PresenceCondition::isSatisfiable() {
    std::string cnf = this->toNegationNormal()->toCnf()->toString();
    std::map<std::string, int> map;
    int vars = 0;

    SATSolver solver;
    std::vector<Lit> clause;
    std::string True = "True";
    map[True] = vars++;

    solver.new_vars(1);
    clause.push_back(Lit(map[True], false));
    solver.add_clause(clause);
    clause.clear();

    bool isNeg = false;
    for(unsigned long i = 0; i < cnf.length(); i++){
        if(cnf[i] == '(' || cnf[i] == ')'){
            // do nothing
        }else if(cnf[i] == ' '){
            // do nothing
        }else if(cnf[i] == '|'){
            i++;
        }else if(cnf[i] == '&'){
            i++;
            solver.add_clause(clause);
            clause.clear();
        }else if(cnf[i] == '~'){
            isNeg = !isNeg;
        }else if(isalpha(cnf[i])){
            std::string name = "";
            while(isalpha(cnf[i])){
                name += cnf[i++];
            }
            if(map.find(name) == map.end()){
                map[name] = vars++;
                solver.new_vars(1);
            }
            clause.push_back(Lit(map[name], isNeg));
            isNeg = false;
        }
    }
    solver.add_clause(clause);
    
    return solver.solve() == l_True;
}

const std::string PresenceCondition::toString() {
    // This should never be called, it should always be overridden
    return "##ERROR##";
}


const std::string True::toString() {
    return "True";
}

const std::string Literal::toString() {
    return this->literal;
}

const std::string And::toString() {
    return "(" + this->left->toString() + " && " + this->right->toString() + ")";
}

And::And(PresenceCondition* left_, PresenceCondition* right_) {
    this->left = left_;
    this->right = right_;
    this->typeOfPC = AND;
}

const std::string Or::toString() {
    return "(" + this->left->toString() + " || " + this->right->toString() + ")";
}

Or::Or(PresenceCondition* left_, PresenceCondition* right_) {
    this->left = left_;
    this->right = right_;
    this->typeOfPC = OR;
}


const std::string Not::toString() {
    return "~" + this->right->toString();
}


PresenceCondition* PresenceCondition::getList(std::vector<bool> declarations, std::vector<std::string> names) {
    // unpacks the vectors from the condition stack to form the current conditional
    if (names.size() == 0) {
        return new True();
    } else if (names.size() == 1) {
        return declarations[0] ?
               static_cast<PresenceCondition*>(new Literal(names[0])) :
               static_cast<PresenceCondition*>(new Not(new Literal(names[0])));
    } else {
        PresenceCondition* pc = declarations[0] ?
                                static_cast<PresenceCondition*>(new Literal(names[0])) :
                                static_cast<PresenceCondition*>(new Not(new Literal(names[0])));

        for (unsigned long i = 1, e = names.size(); i < e; ++i) {
            pc = new And(pc, declarations[i] ?
                         static_cast<PresenceCondition*>(new Literal(names[i])) :
                         static_cast<PresenceCondition*>(new Not(new Literal(names[i]))));
        }
        return pc;
    }

}
PresenceCondition* PresenceCondition::toNegationNormal(){
    // this should never be called
    return this;
}
PresenceCondition* PresenceCondition::toCnf(){
    // this should never be called
    return this;
}

PresenceCondition* Or::toNegationNormal(){
    Or* pc = this;
    pc->left = pc->left->toNegationNormal();
    pc->right = pc->right->toNegationNormal();
    
    if(pc->right->typeOfPC == TRUE){
        return pc->right;
    }
    if(pc->left->typeOfPC == TRUE){
        return pc->left;
    }
    if(pc->left->typeOfPC == NOT){
        if(((Not*)pc->left)->right->typeOfPC == TRUE){
            return pc->right;
        }
    }
    if(pc->right->typeOfPC == NOT){
        if(((Not*)pc->right)->right->typeOfPC == TRUE){
            return pc->left;
        }
    }
    return pc;
}

PresenceCondition* And::toNegationNormal(){
    And* pc = this;
    pc->left = pc->left->toNegationNormal();
    pc->right = pc->right->toNegationNormal();
    if(pc->right->typeOfPC == TRUE){
        return pc->left;
    }
    if(pc->left->typeOfPC == TRUE){
        return pc->right;
    }
    if(pc->left->typeOfPC == NOT){
        if(((Not*)pc->left)->right->typeOfPC == TRUE){
            return pc->left;
        }
    }
    if(pc->right->typeOfPC == NOT){
        if(((Not*)pc->right)->right->typeOfPC == TRUE){
            return pc->right;
        }
    }
    return pc;

}


PresenceCondition* Not::toNegationNormal(){
    Not* pc = this;
    if(pc->right->typeOfPC == NOT){
        return ((Not*)pc->right)->right->toNegationNormal();
    }else if(pc->right->typeOfPC == OR){
        return (new And(new Not(((Or*)pc->right)->left), new Not(((Or*)pc->right)->right)))->toNegationNormal();
    }else if(pc->right->typeOfPC == AND){
        return (new Or(new Not(((And*)pc->right)->left), new Not(((And*)pc->right)->right)))->toNegationNormal();
    }


    return pc;
}

PresenceCondition* True::toNegationNormal(){
    return this;
}

PresenceCondition* Literal::toNegationNormal(){
    return this;
}
PresenceCondition* Or::toCnf(){
    Or* pc = this;
    if(pc->right->typeOfPC == AND){
        PresenceCondition* tmp = pc->right;
        pc->right = pc->left;
        pc->left = tmp;
    }

    // ((a*b) | c) ---> ((a|c) & (b|c))
    if(pc->left->typeOfPC == AND){
        return (new And(new Or(((And*)pc->left)->left, pc->right), new Or(((And*)pc->left)->right, pc->right)))->toCnf();
    }
    return this;
}

PresenceCondition* And::toCnf(){
    And* pc = this;
    pc->left = pc->left->toCnf();
    pc->right = pc->right->toCnf();
    return pc;
}

PresenceCondition* Not::toCnf(){
    return this;
}

PresenceCondition* True::toCnf(){
    return this;
}

PresenceCondition* Literal::toCnf(){
    return this;
}
