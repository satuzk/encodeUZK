
namespace encodeuzk {

template<typename VarAllocator, typename ClauseEmitter>
void forceComparator(VarAllocator &allocator, ClauseEmitter &emitter,
		typename ClauseEmitter::Literal x1,
		typename ClauseEmitter::Literal x2,
		typename ClauseEmitter::Literal y1,
		typename ClauseEmitter::Literal y2) {
	// these clauses represent min(x1, x2) <= y1, max(x1, x2) <= y2
	emit(emitter, { x1.inverse(), y1 });
	emit(emitter, { x2.inverse(), y1 });
	emit(emitter, { x1.inverse(), x2.inverse(), y2 });

	// these clauses represent min(x1, x2) >= y1, max(x1, x2) >= y2
	emit(emitter, { y2.inverse(), x1 });
	emit(emitter, { y2.inverse(), x2 });
	emit(emitter, { y1.inverse(), x1, x2 });
}

template<typename VarAllocator, typename ClauseEmitter>
std::pair<std::vector<typename ClauseEmitter::Literal>,
		std::vector<typename ClauseEmitter::Literal>>
computePwSplit(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename ClauseEmitter::Literal> &ins,
		typename ClauseEmitter::Literal null_lit) {
	assert(ins.size() % 2 == 0);

	std::vector<typename ClauseEmitter::Literal> outs_a;
	std::vector<typename ClauseEmitter::Literal> outs_b;

	for(int i = 0; i < ins.size() / 2; i++) {
		typename ClauseEmitter::Literal a = allocator.allocate().oneLiteral();
		typename ClauseEmitter::Literal b = allocator.allocate().oneLiteral();
		outs_a.push_back(a);
		outs_b.push_back(b);
		forceComparator(allocator, emitter, ins[2 * i], ins[2 * i + 1], a, b);
	}

	return std::make_pair(outs_a, outs_b);
}

template<typename VarAllocator, typename ClauseEmitter>
std::vector<typename ClauseEmitter::Literal>
computePwMerge(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename ClauseEmitter::Literal> &a,
		const std::vector<typename ClauseEmitter::Literal> &b,
		typename ClauseEmitter::Literal null_lit) {
	assert(a.size() > 0);
	assert(a.size() == b.size());

	if(a.size() == 1) {
		std::vector<typename ClauseEmitter::Literal> outs;
		outs.push_back(a.front());
		outs.push_back(b.front());
		return outs;
	}
	if(a.size() % 2 == 1) {
		std::vector<typename ClauseEmitter::Literal> new_a;
		std::vector<typename ClauseEmitter::Literal> new_b;
		for(int i = 0; i < a.size(); i++)
			new_a.push_back(a[i]);
		for(int i = 0; i < a.size(); i++)
			new_b.push_back(b[i]);
		new_a.push_back(null_lit);
		new_b.push_back(null_lit);

		std::vector<typename ClauseEmitter::Literal> outs
				= computePwMerge(allocator, emitter, new_a, new_b, null_lit);
		outs.pop_back();
		outs.pop_back();
		return outs;
	}
	
	assert(a.size() % 2 == 0);
	
	std::vector<typename ClauseEmitter::Literal> even_a;
	std::vector<typename ClauseEmitter::Literal> even_b;
	for(unsigned int i = 0; i < a.size(); i += 2) {
		even_a.push_back(a[i]);
		even_b.push_back(b[i]);
	}

	std::vector<typename ClauseEmitter::Literal> odd_a;
	std::vector<typename ClauseEmitter::Literal> odd_b;
	for(unsigned int i = 1; i < a.size(); i += 2) {
		odd_a.push_back(a[i]);
		odd_b.push_back(b[i]);
	}

	std::vector<typename ClauseEmitter::Literal> even_temps
			= computePwMerge(allocator, emitter, even_a, even_b, null_lit);
	std::vector<typename ClauseEmitter::Literal> odd_temps
			= computePwMerge(allocator, emitter, odd_a, odd_b, null_lit);
	assert(even_temps.size() == a.size());
	assert(odd_temps.size() == a.size());

	// number of bits that actually have to be merged
	int n_merge = a.size() - 1;
	
	std::vector<typename ClauseEmitter::Literal> outs;

	outs.push_back(even_temps.front());
	for(int i = 0; i < n_merge; i++)
		outs.push_back(allocator.allocate().oneLiteral());
	for(int i = 0; i < n_merge; i++)
		outs.push_back(allocator.allocate().oneLiteral());
	outs.push_back(odd_temps.back());
	assert(outs.size() == 2 * a.size());

	for(unsigned int i = 0; i < n_merge; i++)
		forceComparator(allocator, emitter, even_temps[i + 1], odd_temps[i],
				outs[2 * i + 1], outs[2 * i + 2]);

	// additional clauses to improve propagation
//	for(int i = 0; i < outs.size() - 1; i++)
//		emit(emitter, { outs[i], outs[i + 1].inverse() });
	return outs;
}

template<typename VarAllocator, typename ClauseEmitter>
std::vector<typename ClauseEmitter::Literal>
computePwSort(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename ClauseEmitter::Literal> &ins,
		typename ClauseEmitter::Literal null_lit) {
	if(ins.size() == 0)
		return std::vector<typename ClauseEmitter::Literal>();
	if(ins.size() == 1) {
		std::vector<typename ClauseEmitter::Literal> outs;
		outs.push_back(ins[0]);
		return outs;
	}
	if(ins.size() % 2 == 1) {
		std::vector<typename ClauseEmitter::Literal> new_ins;
		for(int i = 0; i < ins.size(); i++)
			new_ins.push_back(ins[i]);
		new_ins.push_back(null_lit);

		std::vector<typename ClauseEmitter::Literal> outs
				= computePwSort(allocator, emitter, new_ins, null_lit);
		outs.pop_back();
		return outs;
	}
	
	std::pair<std::vector<typename ClauseEmitter::Literal>,
				std::vector<typename ClauseEmitter::Literal>> parts
			= computePwSplit(allocator, emitter, ins, null_lit);
		
	std::vector<typename ClauseEmitter::Literal> outs_a
			= computePwSort(allocator, emitter, parts.first, null_lit);
	std::vector<typename ClauseEmitter::Literal> outs_b
			= computePwSort(allocator, emitter, parts.second, null_lit);
	return computePwMerge(allocator, emitter, outs_a, outs_b, null_lit);
}

template<typename VarAllocator, typename ClauseEmitter>
void forceAtLeastPw(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename ClauseEmitter::Literal> &ins, int weight,
		typename ClauseEmitter::Literal null_lit) {
	if(weight <= 0)
		return;
	if(ins.size() < (unsigned int)weight) {
		forceContradiction(allocator, emitter);
		return;
	}
	
	std::vector<typename ClauseEmitter::Literal> outs
			= computePwSort(allocator, emitter, ins, null_lit);
	forceTrue(allocator, emitter, outs[weight - 1]);
}

template<typename VarAllocator, typename ClauseEmitter>
void forceAtMostPw(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename ClauseEmitter::Literal> &ins, int weight,
		typename ClauseEmitter::Literal null_lit) {
	if(weight < 0) {
		forceContradiction(allocator, emitter);
		return;
	}
	if(ins.size() <= (unsigned int)weight)
		return;
	
	std::vector<typename ClauseEmitter::Literal> outs
			= computePwSort(allocator, emitter, ins, null_lit);
	forceFalse(allocator, emitter, outs[weight]);
}

template<typename Weight>
std::vector<int> convertBase(Weight num, const std::vector<int> &base) {
	std::vector<int> products(base.size());
	products[0] = 1;
	for(int i = 1; i < base.size(); i++) {
		products[i] = products[i - 1] * base[i];
	}

	std::vector<int> digits(base.size());
	for(int i = base.size() - 1; i >= 0; i--) {
		Weight k = num / products[i];
		assert(k < std::numeric_limits<int>::max());
		digits[i] = k;
		num -= k * products[i];
	}
	return digits;
}

template<typename Literal>
using SorterLits = std::vector<Literal>;

template<typename Literal>
using SorterNetwork = std::vector<SorterLits<Literal>>;

template<typename VarAllocator, typename ClauseEmitter,
		typename Weight>
SorterNetwork<typename ClauseEmitter::Literal>
computeSorterNetwork(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename ClauseEmitter::Literal> &lits,
		const std::vector<Weight> &weights, const std::vector<int> &base) {
	// we need a literal that is always zero to simplify sorting
	typename ClauseEmitter::Literal null_lit = allocator.allocate().oneLiteral();
	emit(emitter, { null_lit.inverse() });
	
	SorterNetwork<typename ClauseEmitter::Literal> sorters;

	for(int k = 0; k < base.size(); k++) {
		std::vector<typename ClauseEmitter::Literal> ins;

		// add carry bits from previous sorter as input
		if(k > 0) {
			for(int j = base[k] - 1; j < sorters.back().size(); j += base[k])
				ins.push_back(sorters.back()[j]);
		}

		for(int i = 0; i < lits.size(); i++) {
			std::vector<int> weight = convertBase(weights[i], base);
			for(int j = 0; j < weight[k]; j++)
				ins.push_back(lits[i]);
		}

		std::vector<typename ClauseEmitter::Literal> outs
				= computePwSort(allocator, emitter, ins, null_lit);
		std::cout << "c Size of sorter " << k << ": " << outs.size() << std::endl;
		sorters.push_back(outs);
	}

	return sorters;
}

// produces the constraint sorter >= target
template<typename VarAllocator, typename ClauseEmitter>
typename ClauseEmitter::Literal computeSorterGe(VarAllocator &allocator, ClauseEmitter &emitter,
		const SorterLits<typename ClauseEmitter::Literal> &sorter, int target) {
	if(target == 0) {
		typename ClauseEmitter::Variable r = allocator.allocate();

		// trivial case 1: every number is >= 0
		emit(emitter, { r.oneLiteral() });

		return r.oneLiteral();
	}else if(sorter.size() < target) {
		typename ClauseEmitter::Variable r = allocator.allocate();

		// trivial case 2: the sorter is not big enough to reach the limit
		emit(emitter, { r.zeroLiteral() });

		return r.oneLiteral();
	}

	return sorter[target - 1];
}

// produces the constraint sorter % divisor >= target
template<typename VarAllocator, typename ClauseEmitter>
typename ClauseEmitter::Literal computeSorterRemainderGe(VarAllocator &allocator, ClauseEmitter &emitter,
		const SorterLits<typename ClauseEmitter::Literal> &sorter, int divisor, int target) {
	if(target == 0) {
		typename ClauseEmitter::Variable r = allocator.allocate();
	
		// trivial case 1: every number is >= 0
		emit(emitter, { r.oneLiteral() });

		return r.oneLiteral();
	}else if(sorter.size() < target) {
		typename ClauseEmitter::Variable r = allocator.allocate();
	
		// trivial case 2: the sorter is not big enough to reach the limit
		emit(emitter, { r.zeroLiteral() });
		
		return r.oneLiteral();
	}else if(divisor <= target) {
		typename ClauseEmitter::Variable r = allocator.allocate();
	
		// trivial case 3: the modulus is not big enough to reach the limit
		emit(emitter, { r.zeroLiteral() });
		
		return r.oneLiteral();
	}
	
	std::vector<typename ClauseEmitter::Literal> disjunction;
	for(int k = 0; k < sorter.size(); k += divisor) {
		if(k + target - 1 >= sorter.size())
			break;

		if(k + divisor - 1 < sorter.size()) {
			disjunction.push_back(computeAnd(allocator, emitter,
					sorter[k + target - 1], sorter[k + divisor - 1].inverse()));
		}else{
			disjunction.push_back(sorter[k + target - 1]);
		}
	}
	
	return computeOrN(allocator, emitter,
			disjunction.begin(), disjunction.end());
}

// produces the constraint network >= rhs
template<typename VarAllocator, typename ClauseEmitter>
typename ClauseEmitter::Literal computeSorterNetworkGe(VarAllocator &allocator, ClauseEmitter &emitter,
		const SorterNetwork<typename ClauseEmitter::Literal> &network,
		const std::vector<int> &base, const std::vector<int> &rhs, int i) {
	if(i == 0) {
		typename ClauseEmitter::Variable r = allocator.allocate();

		// trivial case: every number is >= 0
		emit(emitter, { r.oneLiteral() });
		
		return r.oneLiteral();
	}

	typename ClauseEmitter::Literal p
			= computeSorterNetworkGe(allocator, emitter, network, base, rhs, i - 1);
	
	typename ClauseEmitter::Literal gt;
	typename ClauseEmitter::Literal ge;
	if(i == base.size()) {
		gt = computeSorterGe(allocator, emitter, network[i - 1], rhs[i - 1] + 1);
		ge = computeSorterGe(allocator, emitter, network[i - 1], rhs[i - 1]);
	}else{
		gt = computeSorterRemainderGe(allocator, emitter, network[i - 1], base[i], rhs[i - 1] + 1);
		ge = computeSorterRemainderGe(allocator, emitter, network[i - 1], base[i], rhs[i - 1]);
	}
	
	return computeOr(allocator, emitter, gt,
			computeAnd(allocator, emitter, ge, p));
}

template<typename VarAllocator, typename ClauseEmitter>
typename ClauseEmitter::Literal computeSorterNetworkGe(VarAllocator &allocator, ClauseEmitter &emitter,
		const SorterNetwork<typename ClauseEmitter::Literal> &network,
		const std::vector<int> &base, const std::vector<int> &rhs) {
	return computeSorterNetworkGe(allocator, emitter, network, base, rhs,
			network.size());
}

} // namespace encodeuzk

