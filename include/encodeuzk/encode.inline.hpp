
namespace encodeuzk {

template<typename VarAllocator>
std::vector<typename VarAllocator::Variable> allocateN(VarAllocator &allocator,
		size_t n) {
	std::vector<typename VarAllocator::Variable> result;
	for(size_t i = 0; i < n; i++)
		result.push_back(allocator.allocate());
	return result;
}

template<typename Variable>
std::vector<typename Variable::Literal> oneLiteralN(const std::vector<Variable> &in) {
	std::vector<typename Variable::Literal> out;
	for(auto it = in.begin(); it != in.end(); ++it)
		out.push_back(it->oneLiteral());
	return out;
}

template<typename Variable>
std::vector<typename Variable::Literal> zeroLiteralN(const std::vector<Variable> &in) {
	std::vector<typename Variable::Literal> out;
	for(auto it = in.begin(); it != in.end(); ++it)
		out.push_back(it->zeroLiteral());
	return out;
}

template<typename ClauseEmitter, typename Container>
void emit(ClauseEmitter &emitter, const Container &container) {
	emitter.emit(container.begin(), container.end());
}
template<typename ClauseEmitter>
void emit(ClauseEmitter &emitter, const std::initializer_list<typename ClauseEmitter::Literal> &container) {
	emitter.emit(container.begin(), container.end());
}

} // namespace encodeuzk

