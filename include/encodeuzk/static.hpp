
namespace encodeuzk {

template<typename BaseDefs>
class StaticLiteral;

template<typename BaseDefs>
class StaticFormula;

template<typename BaseDefs>
class StaticAllocator;

template<typename BaseDefs>
class StaticEmitter;

template<typename BaseDefs>
class StaticVariable {
public:
	typedef typename BaseDefs::LiteralIndex Index;
	typedef StaticLiteral<BaseDefs> Literal;
	
	static StaticVariable<BaseDefs> illegalVar() {
		return StaticVariable<BaseDefs>(-1);
	}
	static StaticVariable<BaseDefs> fromIndex(Index index) {
		return StaticVariable<BaseDefs>(index);
	}
	static StaticVariable<BaseDefs> fromNumber(int64_t number) {
		return StaticVariable<BaseDefs>(number - 1);
	}
	
	StaticVariable() : p_index(-1) { }
	
	Index getIndex() const {
		return p_index;
	}
	int64_t toNumber() const {
		return p_index + 1;
	}
	
	bool operator== (const StaticVariable<BaseDefs> &other) const {
		return p_index == other.p_index;
	}
	bool operator!= (const StaticVariable<BaseDefs> &other) const {
		return p_index != other.p_index;
	}

	StaticLiteral<BaseDefs> zeroLiteral() const;
	StaticLiteral<BaseDefs> oneLiteral() const;
private:
	explicit StaticVariable(Index index) : p_index(index) { }
	
	Index p_index;
};

template <typename BaseDefs>
class StaticLiteral {
public:
	typedef typename BaseDefs::LiteralIndex Index;
	
	static StaticLiteral<BaseDefs> illegalLit() {
		return StaticLiteral<BaseDefs>(-1);
	}
	static StaticLiteral<BaseDefs> fromIndex(Index index) {
		return StaticLiteral<BaseDefs>(index);
	}
	static StaticLiteral<BaseDefs> fromNumber(int64_t number) {
		return number < 0 ? (StaticVariable<BaseDefs>::fromNumber(-number)).zeroLiteral()
				: (StaticVariable<BaseDefs>::fromNumber(number)).oneLiteral();
	}

	StaticLiteral() : p_index(-1) { }
	
	Index getIndex() const {
		return p_index;
	}
	int64_t toNumber() const {
		return isOneLiteral() ? variable().toNumber() : -variable().toNumber();
	}
	
	bool operator== (const StaticLiteral<BaseDefs> &other) const {
		return p_index == other.p_index;
	}
	bool operator!= (const StaticLiteral<BaseDefs> &other) const {
		return p_index != other.p_index;
	}

	StaticVariable<BaseDefs> variable() const {
		return StaticVariable<BaseDefs>::fromIndex(p_index >> 1);
	}
	
	bool isOneLiteral() const {
		return p_index & 1;
	}
	int polarity() const {
		return isOneLiteral() ? 1 : -1;
	}
	StaticLiteral<BaseDefs> inverse() const {
		return StaticLiteral<BaseDefs>::fromIndex(p_index ^ 1);
	}

private:
	explicit StaticLiteral(Index index) : p_index(index) { }
	
	Index p_index;
};

template<typename BaseDefs>
struct StaticDefs {
	typedef StaticVariable<BaseDefs> Variable;
	typedef StaticLiteral<BaseDefs> Literal;
	typedef StaticAllocator<BaseDefs> VarAllocator;
	typedef StaticEmitter<BaseDefs> ClauseEmitter;
};

template<typename BaseDefs>
class StaticAllocator {
public:
	typedef StaticDefs<BaseDefs> Defs;
	typedef typename Defs::Variable Variable;
	typedef typename Defs::Literal Literal;

	StaticAllocator(StaticFormula<BaseDefs> &formula);

	virtual Variable allocate();

private:
	StaticFormula<BaseDefs> &p_formula;
};

template<typename BaseDefs>
class StaticEmitter {
public:
	typedef StaticDefs<BaseDefs> Defs;
	typedef typename Defs::Variable Variable;
	typedef typename Defs::Literal Literal;

	StaticEmitter(StaticFormula<BaseDefs> &formula);

	template<typename Iterator>
	void emit(Iterator begin, Iterator end);
private:
	StaticFormula<BaseDefs> &p_formula;
};

template<typename BaseDefs>
std::ostream &operator<< (std::ostream &stream, const StaticFormula<BaseDefs> &formula);

template<typename BaseDefs>
class StaticFormula {
public:
	typedef StaticDefs<BaseDefs> Defs;
	typedef typename Defs::Variable Variable;
	typedef typename Defs::Literal Literal;
	typedef typename Defs::VarAllocator VarAllocator;
	typedef typename Defs::ClauseEmitter ClauseEmitter;
	
	friend class StaticAllocator<BaseDefs>;
	friend class StaticEmitter<BaseDefs>;
	friend std::ostream &operator<< <> (std::ostream &stream, const StaticFormula<BaseDefs> &formula);

	StaticFormula();

private:
	int p_numVariables;
	int p_numClauses;
	std::vector<int> p_clauses;
};

}; // namespace encodeuzk

