
namespace encodeuzk {

template<typename VarAllocator, typename ClauseEmitter>
void forceContradiction(VarAllocator &allocator, ClauseEmitter &emitter) {
	emit(emitter, { });
}

template<typename VarAllocator, typename ClauseEmitter>
void forceTrue(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal lit) {
	emit(emitter, { lit });
}

template<typename VarAllocator, typename ClauseEmitter>
void forceFalse(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal lit) {
	emit(emitter, { lit.inverse() });
}

template<typename VarAllocator, typename ClauseEmitter>
void forceImplies(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal lit,
		typename VarAllocator::Literal implied) {
	emit(emitter, { lit.inverse(), implied });
}

template<typename VarAllocator, typename ClauseEmitter>
void forceEquivalent(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal a,
		typename VarAllocator::Literal b) {
	forceImplies(allocator, emitter, a, b);
	forceImplies(allocator, emitter, b, a);
}

template<typename VarAllocator, typename ClauseEmitter>
typename VarAllocator::Literal computeOr(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal a,
		typename VarAllocator::Literal b) {
	typename VarAllocator::Variable r = allocator.allocate();

	emit(emitter, { a, b, r.zeroLiteral() });
	emit(emitter, { r.oneLiteral(), a.inverse() });
	emit(emitter, { r.oneLiteral(), b.inverse() });

	return r.oneLiteral();
}

template<typename VarAllocator, typename ClauseEmitter,
		typename Iterator>
typename VarAllocator::Literal computeOrN(VarAllocator &allocator, ClauseEmitter &emitter,
		Iterator begin, Iterator end) {
	typename VarAllocator::Variable r = allocator.allocate();

	std::vector<typename ClauseEmitter::Literal> c(begin, end);
	c.push_back(r.zeroLiteral());
	emitter.emit(c.begin(), c.end());

	for(auto it = begin; it != end; it++)
		emit(emitter, { r.oneLiteral(), it->inverse() });

	return r.oneLiteral();
}

template<typename VarAllocator, typename ClauseEmitter>
typename VarAllocator::Literal computeAnd(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal a,
		typename VarAllocator::Literal b) {
	typename VarAllocator::Variable r = allocator.allocate();

	emit(emitter, { a.inverse(), b.inverse(), r.oneLiteral() });
	emit(emitter, { r.zeroLiteral(), a });
	emit(emitter, { r.zeroLiteral(), b });

	return r.oneLiteral();
}

template<typename VarAllocator, typename ClauseEmitter>
typename VarAllocator::Literal computeXor(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal a,
		typename VarAllocator::Literal b) {
	typename VarAllocator::Variable r = allocator.allocate();

	emit(emitter, { a.inverse(), b.inverse(), r.zeroLiteral() });
	emit(emitter, { a.inverse(), b, r.oneLiteral() });
	emit(emitter, { a, b.inverse(), r.oneLiteral() });
	emit(emitter, { a, b, r.zeroLiteral() });

	return r.oneLiteral();
}

template<typename VarAllocator, typename ClauseEmitter>
std::pair<typename VarAllocator::Literal, typename VarAllocator::Literal>
computeHalfAdd(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal a,
		typename VarAllocator::Literal b) {
	return std::make_pair(computeXor(allocator, emitter, a, b),
		computeAnd(allocator, emitter, a, b));
}

template<typename VarAllocator, typename ClauseEmitter>
std::pair<typename VarAllocator::Literal, typename VarAllocator::Literal>
computeFullAdd(VarAllocator &allocator, ClauseEmitter &emitter,
		typename VarAllocator::Literal a,
		typename VarAllocator::Literal b,
		typename VarAllocator::Literal carry) {
	auto first_add = computeHalfAdd(allocator, emitter, a, b);
	auto second_add = computeHalfAdd(allocator, emitter, first_add.first, carry);
	return std::make_pair(second_add.first,
		computeOr(allocator, emitter, first_add.second, second_add.second));
}

template<typename VarAllocator, typename ClauseEmitter>
std::vector<typename VarAllocator::Literal> computeAddN(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename VarAllocator::Literal> &a,
		const std::vector<typename VarAllocator::Literal> &b) {
	size_t n = a.size();
	assert(n == b.size());
	assert(n > 0);

	std::vector<typename VarAllocator::Literal> r;

	auto single_add = computeHalfAdd(allocator, emitter, a[0], b[0]);
	r.push_back(single_add.first);

	for(size_t i = 1; i < n; i++) {
		single_add = computeFullAdd(allocator, emitter, a[i], b[i], single_add.second);
		r.push_back(single_add.first);
	}

	return r;
}

template<typename VarAllocator, typename ClauseEmitter>
void forceAtMostOne(VarAllocator &allocator, ClauseEmitter &emitter,
		const std::vector<typename ClauseEmitter::Literal> &ins) {
	for(auto i = ins.begin(); i != ins.end(); ++i)
		for(auto j = ins.begin(); j != ins.end(); ++j)
			if(i != j)
				emit(emitter, { i->inverse(), j->inverse() });
}

} // namespace encodeuzk

