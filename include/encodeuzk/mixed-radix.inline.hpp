
#ifndef ENCODEUZK_MIXED_RADIX_HPP
#define ENCODEUZK_MIXED_RADIX_HPP

namespace encodeuzk {

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

class PartialBase {
public:
	int length() const {
		return p_vector.size();
	}
	
	void extend(int p) {
		p_vector.push_back(p);
	}

	int product() const {
		return std::accumulate(p_vector.begin(), p_vector.end(),
				1, std::multiplies<int>());
	}

	int operator[] (int index) const {
		return p_vector[index];
	}

	template<typename Weight>
	std::vector<int> convert(Weight num) const {
		std::vector<int> products(p_vector.size() + 1);
		products[0] = 1;
		for(int i = 0; i < p_vector.size(); i++) {
			products[i + 1] = products[i] * p_vector[i];
		}

		std::vector<int> digits(p_vector.size() + 1);
		for(int i = p_vector.size(); i >= 0; i--) {
			int k = num / products[i];
			digits[i] = k;
			num -= k * products[i];
		}
		return digits;
	}
	
private:
	std::vector<int> p_vector;
};

template<typename Weight>
int cost(const PartialBase &base, const std::vector<Weight> &weights) {
	int sum = 0;
	for(int i = 0; i < weights.size(); i++) {
		std::vector<int> seq = base.convert(weights[i]);
		for(int j = 0; j < base.length() + 1; j++)
			sum += seq[j];
	}
	return sum;
}

// for each base b' extending a base b we have cost(b') >= partial(b)
// i.e. if the partial cost of a base exceeds the cost of the best known
// base we can prun it from the search tree
template<typename Weight>
int partial(const PartialBase &base, const std::vector<Weight> &weights) {
	int sum = 0;
	for(int i = 0; i < weights.size(); i++) {
		std::vector<int> seq = base.convert(weights[i]);
		for(int j = 0; j < base.length(); j++)
			sum += seq[j];
	}
	return sum;
}

// for each base b' extending a base b we have
// cost(b') >= partial(b') + heuristic(b') >= partial(b) + heuristic(b)
// i.e. if the partial cost + heuristic of a base exceed the cost of the
// best known base we can prun it from the search tree
template<typename Weight>
int heuristic(const PartialBase &base, const std::vector<Weight> &weights) {
	int count = 0;
	for(int i = 0; i < weights.size(); i++) {
		if(weights[i] > base.product())
			count++;
	}
	return count;
}

template<typename Weight>
void dfsBase(const PartialBase &current, const std::vector<Weight> &weights, Weight max, PartialBase &best) {
	if(partial(current, weights) + heuristic(current, weights) > cost(best, weights))
		return;
	
	if(cost(current, weights) < cost(best, weights))
		best = current;

	for(int p = 2; true; p++) {
		if(current.product() * p > max)
			break;

		PartialBase next = current;
		next.extend(p);
		dfsBase(next, weights, max, best);
	}
}

template<typename Weight>
std::vector<int> optimalBase(const std::vector<Weight> &weights) {
	PartialBase best;
	if(weights.size() > 0) {
		Weight max = *std::max_element(weights.begin(), weights.end());
		while(!(best.product() * 2 > max))
			best.extend(2);
		
		dfsBase(PartialBase(), weights, max, best);
	}
	
	std::vector<int> seq(best.length() + 1);
	seq[0] = 1;
	std::cout << "c Base:";
	for(int i = 0; i < best.length(); i++) {
		std::cout << " " << best[i];
		seq[i + 1] = best[i];
	}
	std::cout << std::endl;
	return seq;	
}

} // namespace encodeuzk

#endif // ENCODEUZK_MIXED_RADIX_HPP

