#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

class ExtremelyBigInt {
 public:
  ExtremelyBigInt() { assign(""); }
  ExtremelyBigInt(const std::string& EBI) { assign(EBI); }
  ExtremelyBigInt(const unsigned long long& n) {
    this->digiStore = ExtremelyBigInt::Parse(n);
  }

  int digiCount() { return this->toString().length(); }

  std::string toString() const {
    std::string displayStr = "";
    int idx = 0;
    const int digiStoreSize = digiStore.size();
    for (const auto& digi : digiStore) {
      displayStr = std::to_string(digi) + displayStr;
      const int len = displayStr.length();
      if (len % BLOCK_SIZE && idx != digiStoreSize - 1) {
        // clang-format off
        displayStr = ExtremelyBigInt::zeroes(
          BLOCK_SIZE - len % BLOCK_SIZE
        ) + displayStr;
        // clang-format on
      }
      idx++;
    }
    displayStr.erase(0, displayStr.find_first_not_of('0'));
    if (isNegative) {
      displayStr = "-" + displayStr;
    }
    return displayStr;
  }

  ExtremelyBigInt operator+(const ExtremelyBigInt& EBI) {
    ExtremelyBigInt result;
    result.digiStore = this->digiStore;
    int idx = 0;
    for (const auto& digi : EBI.digiStore) {
      result.addBlock(idx++, digi);
    }
    return result;
  }
  ExtremelyBigInt operator+(const long long int& carry) {
    ExtremelyBigInt result;
    result.digiStore = this->digiStore;
    auto parsedCarry = ExtremelyBigInt::Parse(carry);
    int idx = 0;
    for (const auto& carry : parsedCarry) {
      result.addBlock(idx++, carry);
    }
    return result;
  }
  ExtremelyBigInt& operator+=(const ExtremelyBigInt& EBI) {
    int idx = 0;
    for (const auto& digi : EBI.digiStore) {
      this->addBlock(idx++, digi);
    }
    return *this;
  }
  ExtremelyBigInt& operator+=(const long long int& carry) {
    auto parsedCarry = ExtremelyBigInt::Parse(carry);
    int idx = 0;
    for (const auto& carry : parsedCarry) {
      this->addBlock(idx++, carry);
    }
    return *this;
  }

  ExtremelyBigInt operator-(ExtremelyBigInt& EBI) {
    ExtremelyBigInt result;
    result.digiStore = this->digiStore;
    int idx = 0;
    int correction = 1;
    if (this->isNegative && !EBI.isNegative) {
      correction = 1;
      result.isNegative = this->isNegative;
    } else if (this->isNegative && EBI.isNegative) {
      correction = -1;
      if (EBI > *this) {
        result.isNegative = false;
      }
    } else if (!this->isNegative && !EBI.isNegative) {
    }
    for (const auto& borrow : EBI.digiStore) {
      result.addBlock(idx++, -1 * borrow * correction);
    }
    trim(result);
    return result;
  }
  ExtremelyBigInt operator-(const long long int& borrow) {
    ExtremelyBigInt result;
    result.digiStore = this->digiStore;
    auto parsedBorrow = ExtremelyBigInt::Parse(borrow);
    int idx = 0;
    for (const auto& borrow : parsedBorrow) {
      result.addBlock(idx++, -1 * borrow);
    }
    trim(result);
    return result;
  }
  ExtremelyBigInt& operator-=(const ExtremelyBigInt& EBI) {
    int idx = 0;
    for (const auto& borrow : EBI.digiStore) {
      this->addBlock(idx++, -1 * borrow);
    }
    trim(*this);
    return *this;
  }
  ExtremelyBigInt& operator-=(const long long int& borrow) {
    auto parsedBorrow = ExtremelyBigInt::Parse(borrow);
    int idx = 0;
    for (const auto& borrow : parsedBorrow) {
      this->addBlock(idx++, -1 * borrow);
    }
    trim(*this);
    return *this;
  }

  // Toom Cook Algorithm is lit for Multiplication
  // But I ain't implementin that incredible algo
  // with poor code quality (skill issue, fr, fr)
  ExtremelyBigInt operator*(const ExtremelyBigInt& EBI) {
    ExtremelyBigInt result;
    this->painfulMultiply(EBI.digiStore, result);
    ManageSign(EBI, result);
    return result;
  }
  ExtremelyBigInt operator*(const long long int& multiplicant) {
    ExtremelyBigInt result;
    auto parsedMultiplicant = ExtremelyBigInt::Parse(multiplicant);
    this->painfulMultiply(parsedMultiplicant, result);
    ManageSign(multiplicant, result);
    return result;
  }
  ExtremelyBigInt& operator*=(const ExtremelyBigInt& EBI) {
    this->painfulMultiply(EBI.digiStore, *this);
    ManageSign(EBI, *this);
    return *this;
  }
  ExtremelyBigInt& operator*=(const long long int& multiplicant) {
    auto parsedMultiplicant = ExtremelyBigInt::Parse(multiplicant);
    this->painfulMultiply(parsedMultiplicant, *this);
    ManageSign(multiplicant, *this);
    return *this;
  }

  ExtremelyBigInt operator/(ExtremelyBigInt& EBI) {
    ExtremelyBigInt result;
    this->painfulDivision(EBI, result);
    ManageSign(EBI, result);
    return result;
  }
  ExtremelyBigInt operator/(long long int& divisor) {
    ExtremelyBigInt result;
    ExtremelyBigInt d(divisor);
    this->painfulDivision(d, result);
    ManageSign(divisor, result);
    return result;
  }
  ExtremelyBigInt& operator/=(ExtremelyBigInt& EBI) {
    this->painfulDivision(EBI, *this);
    ManageSign(EBI, *this);
    return *this;
  }
  ExtremelyBigInt& operator/=(const long long int& divisor) {
    ExtremelyBigInt d(divisor);
    this->painfulDivision(d, *this);
    ManageSign(divisor, *this);
    return *this;
  }

  ExtremelyBigInt& operator=(const std::string& EBI) {
    assign(EBI);
    return *this;
  }
  ExtremelyBigInt& operator=(const unsigned long long& ull) {
    assign(std::to_string(ull));
    return *this;
  }
  ExtremelyBigInt& operator=(const ExtremelyBigInt& other) {
    if (this != &other) {
      digiStore = other.digiStore;
    }
    return *this;
  }
  bool operator>(const ExtremelyBigInt& EBI) {
    if (EBI.digiStore.size() > this->digiStore.size()) {
      return false;
    } else if (EBI.digiStore.size() < this->digiStore.size()) {
      return true;
    }

    unsigned long long i = digiStore.size();
    bool isGreater = false;
    while (i--) {
      if (digiStore[i] == EBI.digiStore[i]) {
        continue;
      }
      if (digiStore[i] > EBI.digiStore[i]) {
        isGreater = true;
        break;
      }
    }
    return isGreater;
  }
  bool operator<(const ExtremelyBigInt& EBI) const {
    if (EBI.digiStore.size() < this->digiStore.size()) {
      return false;
    } else if (EBI.digiStore.size() > this->digiStore.size()) {
      return true;
    }

    unsigned long long i = digiStore.size();
    bool isLesser = false;
    while (i--) {
      if (digiStore[i] == EBI.digiStore[i]) {
        continue;
      }
      if (digiStore[i] < EBI.digiStore[i]) {
        isLesser = true;
        break;
      }
    }
    return isLesser;
  }
  bool operator==(const ExtremelyBigInt& EBI) {
    if (EBI.digiStore.size() != this->digiStore.size()) {
      return false;
    }
    unsigned long long i = digiStore.size();
    while (i--) {
      if (digiStore[i] == EBI.digiStore[i]) {
        continue;
      }
      return false;
    }
    return true;
  }
  bool operator>=(const ExtremelyBigInt& EBI) {
    return *this > EBI || *this == EBI;
  }
  bool operator<=(const ExtremelyBigInt& EBI) {
    return *this < EBI || *this == EBI;
  }

  friend std::ostream& operator<<(std::ostream&, const ExtremelyBigInt&);

  static int digiCount(size_t n) {
    int count = 1;
    while (n / 10)
      count++, n = n / 10;
    return count;
  }

  static std::string zeroes(size_t n) {
    std::string res = "";
    while (n)
      res += "0", n--;
    return res;
  }

 protected:
  std::vector<unsigned long long> digiStore;
  bool isNegative = false;

  void addBlock(int blockIdx, long long carry) {
    if (blockIdx == digiStore.size()) {
      digiStore.push_back(0);
    } else if (blockIdx > digiStore.size()) {
      return;
    }

    auto& digi = digiStore[blockIdx];
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

  void painfulMultiply(std::vector<unsigned long long> parsedMultiplicant,
                       ExtremelyBigInt& result) {
    int idx = 0;
    for (const auto& digi : parsedMultiplicant) {
      int pow = idx++;
      for (const auto& blockData : digiStore) {
        // BLOCK_SIZE = 18
        // Worst Case: Parsed Size = 18
        // Worst Case for 18x18 digit multiplication: result = 36 Digits
        // thus, break into 9x9 = 18 Digits -> add to result
        const long long TENP9 = std::pow(10, 9);

        long long int res = (digi % TENP9) * (blockData % TENP9);
        ExtremelyBigInt EBIres(std::to_string(res) +
                               ExtremelyBigInt::zeroes(18 * pow));
        result = result + EBIres;

        res = (digi / TENP9) * (blockData % TENP9);
        EBIres = std::to_string(res) + ExtremelyBigInt::zeroes(18 * pow) +
                 ExtremelyBigInt::zeroes(9);
        result = result + EBIres;

        res = (digi % TENP9) * (blockData / TENP9);
        EBIres = std::to_string(res) + ExtremelyBigInt::zeroes(18 * pow) +
                 ExtremelyBigInt::zeroes(9);
        result = result + EBIres;

        res = (digi / TENP9) * (blockData / TENP9);
        EBIres = std::to_string(res) + ExtremelyBigInt::zeroes(18 * (pow + 1));
        result = result + EBIres;
        pow++;
      }
    }
  }

  // https://skanthak.hier-im-netz.de/division.html --> Knuth Algorithm D
  void painfulDivision(ExtremelyBigInt& divisor, ExtremelyBigInt& quotient) {
    if (divisor.digiStore.size() > digiStore.size()) {
      quotient = 0;
      return;
    }

    unsigned long long count = 0;
    unsigned long long divisorLen = divisor.digiStore.size();
    ExtremelyBigInt remainder;
    std::vector<unsigned long long>::iterator it = digiStore.begin();
    int i = 0;

    remainder.digiStore.clear();
    while (std::distance(it, digiStore.end()) >= divisorLen) {
      for (auto& items : std::vector<unsigned long long>(it, it + divisorLen)) {
        remainder.digiStore.push_back(items);
      }

      while (remainder >= divisor) {
        remainder = remainder - divisor;
        count++;
      }
      quotient = quotient + count;
      it += divisorLen;
    }
    if (it != digiStore.end()) {
      remainder.digiStore =
          std::vector<unsigned long long>(it, digiStore.end());
      while (remainder >= divisor) {
        remainder = remainder - divisor;
        count++;
      }
      quotient = quotient + count;
      it += divisorLen;
    }
  }

  // Little Endian stlye assignment
  void assign(std::string EBI) {
    digiStore.clear();
    ExtremelyBigInt::clean(EBI);
    if (EBI == "") {
      digiStore.push_back(0);
      return;
    }
    if (EBI[0] == '-') {
      isNegative = true;
      EBI = EBI.substr(1);
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

  void trim(ExtremelyBigInt& result) {
    while (!result.digiStore.empty() && result.digiStore.back() == 0) {
      result.digiStore.pop_back();
    }
  }

  void ManageSign(const ExtremelyBigInt& EBI, ExtremelyBigInt& result) {
    if ((EBI.isNegative && this->isNegative) ||
        (!EBI.isNegative && !this->isNegative)) {
      result.isNegative = false;
    } else {
      result.isNegative = true;
    }
  }
  void ManageSign(const long long int& multiplicant, ExtremelyBigInt& result) {
    if ((multiplicant < 0 && this->isNegative) ||
        (multiplicant > 0 && !this->isNegative)) {
      result.isNegative = false;
    } else {
      result.isNegative = true;
    }
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
  static std::string& clean(std::string& str) {
    str.erase(str.find_last_not_of(' ') + 1);
    str.erase(0, str.find_first_not_of(' '));
    return str;
  }
};

std::ostream& operator<<(std::ostream& os, const ExtremelyBigInt& EBI) {
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

  i = "12345678910111213141511617";
  j = "5678";
  k = i * j;
  std::cout << i << " " << k << "\n";
  k = i * i;
  std::cout << i << " " << k << "\n";

  ExtremelyBigInt l(
      "12345678910111213141516171819120212223242526272829307474747477777777");
  std::cout << "And now, for the destruction of cpu:\n";
  k = l * l * l * l;
  std::cout << k << "\t\t" << k.digiCount() << "\n\n";

  std::cout << "Going division\n";
  ExtremelyBigInt b("24000000000000000000"), c("2400000000000000000");
  std::cout << c << "m\n";
  std::cout << b / c;

  std::cout << "\nSubin:\n";
  b = "1234567891234567891";
  c = "1000000000000000000";
  std::cout << b - c << "\n";

  ExtremelyBigInt z("-1000000000000"), y("1000000000");
  std::cout << "The new z: " << z << "\t" << y << "\t";
  std::cout << (z * y) << "\t" << (z / y) << "\t" << (z - y);

  return 0;
}
