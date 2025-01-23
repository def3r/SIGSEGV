#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

class ExtremelyBigInt {
public:
  ExtremelyBigInt(std::string EBI) {
    ExtremelyBigInt::clean(EBI);
    if (EBI == "") {
      digiStore.push_back(0);
      return;
    }

    // clang-format off
    while (EBI.size() > BLOCK_SIZE) {
      digiStore.push_back(
        std::stoull(EBI.substr(EBI.length() - BLOCK_SIZE))
      );
      EBI = EBI.substr(0, EBI.length() - BLOCK_SIZE);
    }
    digiStore.push_back(std::stoull(EBI));
    // clang-format on
  }

  int digiCount() { return this->toString().length(); }

  std::string toString() {
    std::string displayStr = "";
    for (auto &digi : digiStore) {
      displayStr = std::to_string(digi) + displayStr;
    }
    return displayStr;
  }

  ExtremelyBigInt operator+(ExtremelyBigInt EBI) {
    ExtremelyBigInt result(this->toString());
    int idx = 0;
    for (const auto &digi : EBI.digiStore) {
      result.addBlock(idx++, digi);
    }
    return result;
  }
  ExtremelyBigInt operator+(long long int carry) {
    ExtremelyBigInt result(this->toString());
    auto parsedCarry = ExtremelyBigInt::Parse(carry);
    int idx = 0;
    for (const auto &carry : parsedCarry) {
      result.addBlock(idx++, carry);
    }
    return result;
  }

  ExtremelyBigInt operator-(ExtremelyBigInt EBI) {
    ExtremelyBigInt result(this->toString());
    int idx = 0;
    for (const auto &borrow : EBI.digiStore) {
      result.addBlock(idx++, -1 * borrow);
    }
    return result;
  }
  ExtremelyBigInt operator-(long long int borrow) {
    ExtremelyBigInt result(this->toString());
    auto parsedBorrow = ExtremelyBigInt::Parse(borrow);
    int idx = 0;
    for (const auto &borrow : parsedBorrow) {
      result.addBlock(idx++, -1 * borrow);
    }
    return result;
  }

  friend std::ostream &operator<<(std::ostream &, ExtremelyBigInt &);

  static int digiCount(size_t n) {
    int count = 1;
    while (n / 10)
      count++, n = n / 10;
    return count;
  }

protected:
  std::vector<unsigned long long> digiStore;

  void addBlock(int blockIdx, long long carry) {
    if (blockIdx == digiStore.size()) {
      digiStore.push_back(0);
    } else if (blockIdx > digiStore.size()) {
      return;
    }

    auto &digi = digiStore[blockIdx];
    unsigned long long temp = digi + carry;
    // OVERFLOW
    if (temp > ExtremelyBigInt::LIMIT) {
      if (carry < 0) {
        digi = TENP18 + digi + carry;
        addBlock(++blockIdx, -1);
        return;
      }
      digi = temp % TENP18;
      carry = temp / TENP18;
      addBlock(++blockIdx, carry);
      return;
    }
    digi = temp;
  }

  static inline int BLOCK_SIZE = 18;
  static inline long long LIMIT = 999999999999999999;
  static inline long long TENP18 = std::pow(10, BLOCK_SIZE);
  static std::vector<unsigned long long> Parse(long long src) {
    std::vector<unsigned long long> blocks;
    while (src % TENP18) {
      blocks.push_back(src % TENP18);
      src /= TENP18;
    }
    return blocks;
  }
  static std::string &clean(std::string &str) {
    str.erase(str.find_last_not_of(' ') + 1);
    str.erase(0, str.find_first_not_of(' '));
    return str;
  }
};

std::ostream &operator<<(std::ostream &os, ExtremelyBigInt &EBI) {
  os << EBI.toString();
  return os;
}

int main() {

  long long int t = 9223372036854775800;

  std::cout << "digi count: " << ExtremelyBigInt::digiCount(ULONG_MAX) << "\n";

  ExtremelyBigInt i = ExtremelyBigInt("18996744073709551615");
  std::cout << i << "; digi count: " << i.digiCount() << "\n\n";
  i = i + 20;
  std::cout << i << "\n";
  i = i + t;
  std::cout << i << "\n";
  i = i - t;
  std::cout << i << "\n";
  i = i - 20;
  std::cout << i << "\n\n";

  i = i + i;
  std::cout << i << "\n";
  ExtremelyBigInt j("1234567891011121314151617181912021222324252627282930");
  std::cout << j << "\n\n";

  ExtremelyBigInt k = i + j;
  std::cout << k << "\n";
  k = j - i;
  std::cout << k << "\n";

  return 0;
}
