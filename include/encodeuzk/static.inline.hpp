
namespace encodeuzk {

template <typename BaseDefs>
StaticLiteral<BaseDefs> StaticVariable<BaseDefs>::zeroLiteral() const {
	return StaticLiteral<BaseDefs>::fromIndex(p_index << 1);
}
template <typename BaseDefs>
StaticLiteral<BaseDefs> StaticVariable<BaseDefs>::oneLiteral() const {
	return StaticLiteral<BaseDefs>::fromIndex((p_index << 1) + 1);
}

template<typename BaseDefs>
StaticAllocator<BaseDefs>::StaticAllocator(StaticFormula<BaseDefs> &formula)
		: p_formula(formula) { }

template<typename BaseDefs>
typename StaticDefs<BaseDefs>::Variable StaticAllocator<BaseDefs>::allocate() {
	p_formula.p_numVariables++;
	return Variable::fromNumber(p_formula.p_numVariables);
}

template<typename BaseDefs>
StaticEmitter<BaseDefs>::StaticEmitter(StaticFormula<BaseDefs> &formula)
		: p_formula(formula) { }

template<typename BaseDefs>
template<typename Iterator>
void StaticEmitter<BaseDefs>::emit(Iterator begin, Iterator end) {
	for(auto it = begin; it != end; ++it)
		p_formula.p_clauses.push_back(it->toNumber());
	p_formula.p_clauses.push_back(0);
	p_formula.p_numClauses++;
}

template<typename BaseDefs>
std::ostream &operator<< (std::ostream &stream, const StaticFormula<BaseDefs> &formula) {
	stream << "p cnf " << formula.p_numVariables << " " << formula.p_numClauses << std::endl;
	bool first_literal = true;
	for(auto it = formula.p_clauses.begin(); it != formula.p_clauses.end(); ++it) {
		if(!first_literal)
			stream << ' ';
		if(*it == 0) {
			stream << '0' << std::endl;
			first_literal = true;
		}else{
			stream << *it;
			first_literal = false;
		}
	}
}

template<typename BaseDefs>
StaticFormula<BaseDefs>::StaticFormula()
		: p_numVariables(0), p_numClauses(0) { }

} // namespace encodeuzk

