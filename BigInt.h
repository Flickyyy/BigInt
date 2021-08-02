#pragma once
#include <algorithm>
#include <exception>
#include <sstream>
#include <utility>
#include <fstream>
#include <vector>
#define ALL(cont) cont.begin(), cont.end()
using namespace std;

template <class _Ty>
void push_front(vector<_Ty>& vec, const _Ty val) {
	vector<_Ty> vec2(vec.size() + 1);
	vec2[0] = val;
	for (size_t i = 0; i < vec.size(); i++) {
		vec2[i + 1] = vec[i];
	}
	vec = move(vec2);
}

class BigInt {
public:
	BigInt() : _is_neg{ false } {}
	BigInt(int64_t val) {
		*this = BigInt(to_string(val));
	}
	BigInt(const string& str) {
		istringstream iss(str);
		iss >> *this;
	}
	friend BigInt abs(const BigInt& val) {
		return val < 0 ? -val : val;
	}
	friend BigInt operator-(BigInt val) {
		val._is_neg = !val._is_neg;
		return move(val);
	}
	friend istream& operator>>(istream& in, BigInt& bi) {
		string str; in >> str;
		if (!all_of(next(str.begin()), str.end(), isdigit) or str[0] == '-' and str.length() == 1
			or (str[0] > '9' or str[0] < '0') and str[0] != '-')
			throw runtime_error("Incorrect format of BigInt\n");
		size_t a;
		if (str[0] == '-') {
			auto it = find_if(next(str.begin()), str.end(), [](char ch) {return ch != '0'; });
			if (it == str.end()) {
				bi._is_neg = false;
				bi._data = { 0 };
				return in;
			}
			a = distance(str.begin(), it);
			bi._is_neg = true;
		}
		else {
			auto it = find_if(ALL(str), [](char ch) {return ch != '0'; });
			if (it == str.end()) {
				bi._is_neg = false;
				bi._data = { 0 };
				return in;
			}
			a = distance(str.begin(), it);
			bi._is_neg = false;
		}
		bi._data.reserve(str.length() - a);
		bi._data.clear();
		for (size_t i = a; i < str.length(); i++) {
			bi._data.push_back(str[i] - '0');
		}

		return in;
	}
	friend ostream& operator<<(ostream& out, const BigInt& bi) {
		if (bi._is_neg)
			out << '-';
		for (auto num : bi._data) {
			out << num;
		}

		return out;
	}
	friend BigInt operator+(const BigInt& lhs, const BigInt& rhs) {
		if (lhs._is_neg == rhs._is_neg) {
			BigInt result;
			result._data.resize(max(lhs._data.size(), rhs._data.size()), 0);
			for (size_t i = 0; i < lhs._data.size(); i++) {
				result._data[result._data.size() - 1 - i] += lhs._data[lhs._data.size() - 1 - i];
			}
			for (size_t i = 0; i < rhs._data.size(); i++) {
				result._data[result._data.size() - 1 - i] += rhs._data[rhs._data.size() - 1 - i];
			}
			result.align();
			result._is_neg = lhs._is_neg;
			return result;
		}
		else if (lhs._is_neg) {
			BigInt lhs_copy = lhs;
			lhs_copy._is_neg = false;
			return rhs - lhs_copy;
		}
		else if (rhs._is_neg) {
			BigInt rhs_copy = rhs;
			rhs_copy._is_neg = false;
			return lhs - rhs_copy;
		}
	}
	friend BigInt operator+=(BigInt& lhs, const BigInt& rhs) {
		lhs = lhs + rhs;
		return lhs;
	}
	BigInt& operator++() {
		*this += 1;
		return *this;
	}
	BigInt operator++(int) {
		*this += 1;
		return *this - 1;
	}
	friend BigInt operator-(const BigInt& lhs, const BigInt& rhs) {
		if (!lhs._is_neg and !rhs._is_neg) {
			BigInt result;
			if (lhs >= rhs) {
				result._data.resize(max(lhs._data.size(), rhs._data.size()), 0);
				for (size_t i = 0; i < lhs._data.size(); i++) {
					result._data[result._data.size() - 1 - i] += lhs._data[lhs._data.size() - 1 - i];
				}
				for (size_t i = 0; i < rhs._data.size(); i++) {
					result._data[result._data.size() - 1 - i] -= rhs._data[rhs._data.size() - 1 - i];
					if (result._data[result._data.size() - 1 - i] < 0) {
						result._data[result._data.size() - 1 - i] += 10;
						result._data[result._data.size() - 2 - i] -= 1;
					}
				}
				auto it = find_if(ALL(result._data), [](auto val) {return val != 0; });
				result._data.erase(result._data.begin(), it);
				if (result._data.size() == 0)
					result._data = { 0 };
				result.align();
				result._is_neg = false;
			}
			else {
				result = rhs - lhs;
				result._is_neg = true;
			}
			return result;
		}
		else if (!lhs._is_neg and rhs._is_neg) {
			BigInt rhs_copy = rhs;
			rhs_copy._is_neg = false;
			return lhs + rhs_copy;
		}
		else if (lhs._is_neg and !rhs._is_neg) {
			BigInt lhs_copy = lhs, result;
			lhs_copy._is_neg = false;
			result = lhs_copy + rhs;
			result._is_neg = true;
			return result;
		}
		else if (lhs._is_neg and rhs._is_neg) {
			BigInt lhs_copy = lhs, rhs_copy = rhs;
			lhs_copy._is_neg = false;
			rhs_copy._is_neg = false;
			return rhs_copy - lhs_copy;
		}
	}
	friend BigInt operator-=(BigInt& lhs, const BigInt& rhs) {
		lhs = lhs - rhs;
		return lhs;
	}
	BigInt& operator--() {
		*this -= 1;
		return *this;
	}
	BigInt operator--(int) {
		*this -= 1;
		return *this + 1;
	}
	friend BigInt operator*(const BigInt& lhs, const BigInt& rhs) {
		if (lhs == 0 or rhs == 0)
			return 0;
		const BigInt& max = (lhs._data.size() > rhs._data.size() ? lhs : rhs),
			& min = (lhs._data.size() <= rhs._data.size() ? lhs : rhs);
		BigInt sum, val;
		for (size_t i = 0; i < max._data.size(); i++) {
			if (max._data[max._data.size() - i - 1] == 0)
				continue;
			val = min;
			for (auto& e : val._data)
				e *= max._data[max._data.size() - i - 1];
			val.align();
			for (size_t j = 0; j < i; j++)
				val._data.push_back(0);
			sum += val;
		}
		sum._is_neg = lhs._is_neg xor rhs._is_neg;
		return sum;
	}
	friend BigInt operator*=(BigInt& lhs, const BigInt& rhs) {
		lhs = lhs * rhs;
		return lhs;
	}
	friend BigInt operator/(const BigInt& lhs, const BigInt& rhs) {
		if (rhs == 0 or lhs == 0 or abs(lhs) < abs(rhs))
			return 0;
		else if (lhs == rhs)
			return 1;
		BigInt result = 0, step = 1;
		while (abs((result + step) * rhs) <= abs(lhs)) {
			result += step;
			step *= 2;
		}
		step /= 2;
		while (step >= 1) {
			while (abs((result + step) * rhs) <= abs(lhs))
				result += step;
			step /= 2;
		}
		result._is_neg = lhs._is_neg xor rhs._is_neg;
		return result;
	}
	friend BigInt operator/=(BigInt& lhs, const BigInt& rhs) {
		lhs = lhs / rhs;
		return lhs;
	}
	friend BigInt operator%(const BigInt& lhs, const BigInt& rhs) {
		return lhs - lhs / rhs * rhs;
	}
	friend BigInt operator%=(BigInt& lhs, const BigInt& rhs) {
		lhs = lhs % rhs;
		return lhs;
	}
	friend BigInt operator^(const BigInt& lhs, const BigInt& rhs) {
		if (rhs == 0) {
			return lhs < 0 ? -1 : 1;
		}
		else if (rhs < 0) {
			return 0;
		}
		else {
			BigInt result = 1;
			for (BigInt i = 0; i < rhs; i++)
				result *= lhs;
			return result;
		}
	}
	friend bool operator<(const BigInt& lhs, const BigInt& rhs) {
		if (lhs._is_neg != rhs._is_neg) {
			return lhs._is_neg;
		}
		else if (lhs._is_neg == false) {
			if (lhs._data.size() != rhs._data.size()) return lhs._data.size() < rhs._data.size();
			for (size_t i = 0; i < lhs._data.size(); i++) {
				if (lhs._data[i] != rhs._data[i]) return lhs._data[i] < rhs._data[i];
			}
		}
		else if (lhs._is_neg == true) {
			if (lhs._data.size() != rhs._data.size()) return lhs._data.size() > rhs._data.size();
			for (size_t i = 0; i < lhs._data.size(); i++) {
				if (lhs._data[i] != rhs._data[i]) return lhs._data[i] > rhs._data[i];
			}
		}
		return false;
	}
	friend bool operator>(const BigInt& lhs, const BigInt& rhs) {
		return !(lhs < rhs) and !(lhs == rhs);
	}
	friend bool operator==(const BigInt& lhs, const BigInt& rhs) {
		if (lhs._data.size() != rhs._data.size()) return false;
		if (lhs._is_neg != rhs._is_neg) return false;
		for (size_t i = 0; i < lhs._data.size(); i++) {
			if (lhs._data[i] != rhs._data[i])
				return false;
		}
		return true;
	}
	friend bool operator!=(const BigInt& lhs, const BigInt& rhs) {
		return not(lhs == rhs);
	}
	friend bool operator<=(const BigInt& lhs, const BigInt& rhs) {
		return lhs < rhs or lhs == rhs;
	}
	friend bool operator>=(const BigInt& lhs, const BigInt& rhs) {
		return lhs > rhs or lhs == rhs;
	}
private:
	void align() {
		for (size_t i = _data.size() - 1; i > 0; i--) {
			_data[i - 1] += _data[i] / 10;
			_data[i] %= 10;
		}
		if (_data[0] > 9) {
			push_front(_data, static_cast<int16_t> (_data[0] / 10));
			_data[1] %= 10;
		}
	}
	vector<int16_t> _data;
	bool _is_neg;
};
BigInt operator""i(const char* str, size_t) {
	return { str };
}
