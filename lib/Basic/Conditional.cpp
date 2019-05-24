#include "clang/Basic/Conditional.h"
#include <ctype.h>
#include <cryptominisat5/cryptominisat.h>

using namespace Variability;

const std::string NOT_SYM = "~";
const std::string AND_SYM = "&&";
const std::string OR_SYM = "||";


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
    //return this->solve_map[other->toString()][2] && !this->solve_map[other->toString()][3];
    return (new Variability::And(new Variability::Not(this), other))->isSatisfiable();
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
    if(toString() == other->toString()){
        bool* ans = new bool[4];
        ans[0] = true; // parser implies token
        ans[1] = false; // parser implies not token
        ans[2] = false; // not (token implies parser)
        ans[3] = false; // not (parser implies token)
        this->solve_map[other->toString()] = ans;
        return;
    }
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

#if 0
    printf("Solve:\n");
    printf("Parser: %s, Tok: %s\n", toString().c_str(), other->toString().c_str());
    printf("Equ_1: %s\n", equ_1->toNegationNormal()->toCnf()->toString().c_str());
    printf("Equ_2: %s\n", equ_2->toNegationNormal()->toCnf()->toString().c_str());
    printf("Equ_3: %s\n", equ_3->toNegationNormal()->toCnf()->toString().c_str());
    printf("Equ_4: %s\n", equ_4->toNegationNormal()->toCnf()->toString().c_str());
    printf("Results: %d %d %d %d\n", ans[0], ans[1], ans[2], ans[3]);
#endif



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
    clause.clear();

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
    return (this->left->typeOfPC == OR ? "(":"") + this->left->toString()
        + (this->left->typeOfPC == OR ? ")":"") + " "+ AND_SYM +" "
        + (this->right->typeOfPC == OR ? "(":"") + this->right->toString()
        + (this->right->typeOfPC == OR ? ")":"");
}

const std::string Or::toString() {
    return this->left->toString() + " "+ OR_SYM +" " + this->right->toString();
}

const std::string Not::toString() {
    if(this->right->typeOfPC == OR
            || this->right->typeOfPC == AND){
        return NOT_SYM+"(" + this->right->toString() +")";
    }

    return NOT_SYM + this->right->toString();
}

And::And(PresenceCondition* left_, PresenceCondition* right_) {
    this->left = left_;
    this->right = right_;
    this->typeOfPC = AND;
}


Or::Or(PresenceCondition* left_, PresenceCondition* right_) {
    this->left = left_;
    this->right = right_;
    this->typeOfPC = OR;
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
    pc->left = pc->left->toCnf();
    pc->right = pc->right->toCnf();
    if(pc->right->typeOfPC == AND){
        PresenceCondition* tmp = pc->right;
        pc->right = pc->left;
        pc->left = tmp;
    }

    // ((a&b) | c) ---> ((a|c) & (b|c))
    if(pc->left->typeOfPC == AND){
        const auto a = ((And*)pc->left)->left->toCnf();
        const auto b = ((And*)pc->left)->right->toCnf();
        const auto c = pc->right->toCnf();
        return (new And(new Or(a, c), new Or(b, c)))->toCnf();
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
