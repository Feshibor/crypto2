#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Byte = unsigned char;
using Bytes = std::vector<Byte>;
using U64 = uint64_t;

Bytes permute(const Bytes& input, const std::vector<int>& table, bool fromHigh = true, int first = 1) {
    if (first != 0 && first != 1) throw std::runtime_error("first must be 0 or 1");
    Bytes out((table.size() + 7) / 8, 0);
    for (size_t i = 0; i < table.size(); ++i) {
        int bit = table[i] - first;
        if (bit < 0 || bit >= static_cast<int>(input.size() * 8))
            throw std::runtime_error("permutation index out of range");
        int source = fromHigh ? 7 - bit % 8 : bit % 8;
        int target = fromHigh ? 7 - i % 8 : i % 8;
        out[i / 8] |= ((input[bit / 8] >> source) & 1) << target;
    }
    return out;
}

U64 number(const Bytes& b) {
    U64 x = 0;
    for (Byte v : b) x = (x << 8) | v;
    return x;
}
Bytes bytes(U64 x, int n) {
    Bytes b(n);
    for (int i = n - 1; i >= 0; --i) { b[i] = static_cast<Byte>(x); x >>= 8; }
    return b;
}
U64 bits(U64 x, int size, const int* table, int count) {
    U64 y = 0;
    for (int i = 0; i < count; ++i) y = (y << 1) | ((x >> (size - table[i])) & 1);
    return y;
}

const int IP[64] = {
    58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,62,54,46,38,30,22,14,6,
    64,56,48,40,32,24,16,8,57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7};
const int FP[64] = {
    40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,38,6,46,14,54,22,62,30,
    37,5,45,13,53,21,61,29,36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25};
const int PC1[56] = {
    57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,35,27,19,11,3,
    60,52,44,36,63,55,47,39,31,23,15,7,62,54,46,38,30,22,14,6,61,53,45,37,
    29,21,13,5,28,20,12,4};
const int PC2[48] = {
    14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,26,8,16,7,27,20,13,2,
    41,52,31,37,47,55,30,40,51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32};
const int EXPAND[48] = {
    32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,12,13,14,15,16,17,
    16,17,18,19,20,21,20,21,22,23,24,25,24,25,26,27,28,29,28,29,30,31,32,1};
const int P[32] = {
    16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,
    2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25};
const int SHIFTS[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
const int S[8][64] = {
    {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},
    {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9},
    {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12},
    {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14},
    {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3},
    {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13},
    {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12},
    {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
};

struct KeySchedule {
    virtual std::vector<Bytes> expand(const Bytes& key) const = 0;
    virtual ~KeySchedule() = default;
};
struct RoundFunction {
    virtual Bytes run(const Bytes& half, const Bytes& key) const = 0;
    virtual ~RoundFunction() = default;
};
struct BlockCipher {
    virtual int blockSize() const = 0;
    virtual void setKey(const Bytes& key) = 0;
    virtual Bytes encrypt(const Bytes& block) const = 0;
    virtual Bytes decrypt(const Bytes& block) const = 0;
    virtual ~BlockCipher() = default;
};

class Feistel : public BlockCipher {
protected:
    const KeySchedule& schedule;
    const RoundFunction& round;
    int size;
    std::vector<Bytes> keys;
    Bytes process(const Bytes& input, bool reverse) const {
        if (static_cast<int>(input.size()) != size || keys.empty())
            throw std::runtime_error("wrong block size or key not set");
        int half = size / 2;
        Bytes left(input.begin(), input.begin() + half);
        Bytes right(input.begin() + half, input.end());
        for (size_t i = 0; i < keys.size(); ++i) {
            const Bytes& k = keys[reverse ? keys.size() - i - 1 : i];
            Bytes f = round.run(right, k);
            if (f.size() != right.size()) throw std::runtime_error("wrong round function size");
            for (int j = 0; j < half; ++j) {f[j] ^= left[j];}
            left = right;
            right = f;
        }
        right.insert(right.end(), left.begin(), left.end());
        return right;
    }
public:
    Feistel(const KeySchedule& k, const RoundFunction& f, int blockSize)
        : schedule(k), round(f), size(blockSize) {}
    int blockSize() const override { return size; }
    void setKey(const Bytes& key) override { keys = schedule.expand(key); }
    Bytes encrypt(const Bytes& block) const override { return process(block, false); }
    Bytes decrypt(const Bytes& block) const override { return process(block, true); }
};

class DESKeys : public KeySchedule {
public:
    std::vector<Bytes> expand(const Bytes& key) const override {
        if (key.size() != 8) throw std::runtime_error("DES key must be 8 bytes");
        U64 k = bits(number(key), 64, PC1, 56);
        uint32_t c = static_cast<uint32_t>(k >> 28);
        uint32_t d = static_cast<uint32_t>(k & 0xfffffff); 
        std::vector<Bytes> result;
        for (int shift : SHIFTS) {
            c = ((c << shift) | (c >> (28 - shift))) & 0xfffffff;
            d = ((d << shift) | (d >> (28 - shift))) & 0xfffffff;
            result.push_back(bytes(bits((U64(c) << 28) | d, 56, PC2, 48), 6));
        }
        return result;
    }
};
class DESRound : public RoundFunction {
public:
    Bytes run(const Bytes& half, const Bytes& key) const override {
        if (half.size() != 4 || key.size() != 6) throw std::runtime_error("wrong DES round size");
        U64 x = bits(number(half), 32, EXPAND, 48) ^ number(key);
        uint32_t y = 0;
        for (int i = 0; i < 8; ++i) {
            int v = (x >> (42 - 6 * i)) & 63;
            int row = ((v & 32) >> 4) | (v & 1);
            y = (y << 4) | S[i][row * 16 + ((v >> 1) & 15)];
        }
        return bytes(bits(y, 32, P, 32), 4);
    }
};
class DES : public Feistel {
    static const DESKeys& keySchedule() { static DESKeys x; return x; }
    static const DESRound& roundFunction() { static DESRound x; return x; }
public:
    DES() : Feistel(keySchedule(), roundFunction(), 8) {}
    Bytes encrypt(const Bytes& b) const override {
        if (b.size() != 8) throw std::runtime_error("DES block must be 8 bytes");
        Bytes t = permute(b, std::vector<int>(IP, IP + 64));
        return permute(process(t, false), std::vector<int>(FP, FP + 64));
    }
    Bytes decrypt(const Bytes& b) const override {
        if (b.size() != 8) throw std::runtime_error("DES block must be 8 bytes");
        Bytes t = permute(b, std::vector<int>(IP, IP + 64));
        return permute(process(t, true), std::vector<int>(FP, FP + 64));
    }
};

class DEALKeys : public KeySchedule {
public:
    std::vector<Bytes> expand(const Bytes& key) const override {
        if (key.size() != 16 && key.size() != 24 && key.size() != 32)
            throw std::runtime_error("DEAL key must be 16, 24 or 32 bytes");
        DES des;
        des.setKey(Bytes{0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF});
        int parts = static_cast<int>(key.size() / 8);
        int rounds = parts == 4 ? 8 : 6;
        U64 previous = 0;
        std::vector<Bytes> result;
        for (int i = 0; i < rounds; ++i) {
            Bytes part(key.begin() + (i % parts) * 8, key.begin() + (i % parts + 1) * 8);
            U64 value = number(part) ^ previous;
            if (i >= parts) value ^= U64(1) << (i - parts);
            Bytes next = des.encrypt(bytes(value, 8));
            previous = number(next);
            result.push_back(next);
        }
        return result;
    }
};
class DESAdapter : public RoundFunction {
public:
    Bytes run(const Bytes& half, const Bytes& key) const override {
        DES des;
        des.setKey(key);
        return des.encrypt(half);
    }
};
class DEAL : public Feistel {
    static const DEALKeys& keySchedule() { static DEALKeys x; return x; }
    static const DESAdapter& roundFunction() { static DESAdapter x; return x; }
public:
    DEAL() : Feistel(keySchedule(), roundFunction(), 16) {}
    Bytes encrypt(const Bytes& block) const override {
        Bytes result = process(block, false);
        std::rotate(result.begin(), result.begin() + 8, result.end());
        return result;
    }
    Bytes decrypt(const Bytes& block) const override {
        if (block.size() != 16) throw std::runtime_error("DEAL block must be 16 bytes");
        Bytes swapped = block;
        std::rotate(swapped.begin(), swapped.begin() + 8, swapped.end());
        return process(swapped, true);
    }
};

enum class Mode { ECB, CBC, PCBC, CFB, OFB, CTR, RandomDelta };
enum class Padding { Zeros, ANSI_X923, PKCS7, ISO_10126 };

Bytes xorBytes(Bytes a, const Bytes& b) {
    for (size_t i = 0; i < a.size(); ++i) a[i] ^= b[i];
    return a;
}
Bytes readFile(const std::string& name) {
    std::ifstream f(name, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open input file: " + name);
    return Bytes(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}
void writeFile(const std::string& name, const Bytes& data) {
    std::ofstream f(name, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open output file: " + name);
    f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!f) throw std::runtime_error("cannot write output file: " + name);
}

class Context {
    BlockCipher& cipher;
    Mode mode;
    Padding padding;
    Bytes iv;
    U64 delta;
    void addPadding(Bytes& data) const {
        int n = cipher.blockSize();
        int count = n - static_cast<int>(data.size() % n);
        if (padding == Padding::Zeros && count == n) return;
        Byte fill = padding == Padding::PKCS7 ? static_cast<Byte>(count) : 0;
        for (int i = 0; i < count; ++i) {
            if (padding == Padding::ISO_10126 && i < count - 1) {
                static std::random_device rng;
                fill = static_cast<Byte>(rng());
            }
            data.push_back(i == count - 1 && padding != Padding::Zeros ? static_cast<Byte>(count) : fill);
        }
    }
    void removePadding(Bytes& data) const {
        if (padding == Padding::Zeros) {
            while (!data.empty() && data.back() == 0) data.pop_back();
            return; 
        }
        if (data.empty()) throw std::runtime_error("missing padding");
        int count = data.back(), n = cipher.blockSize();
        if (count < 1 || count > n || count > static_cast<int>(data.size()))
            throw std::runtime_error("invalid padding or wrong key");
        if (padding == Padding::PKCS7 || padding == Padding::ANSI_X923) {
            for (int i = 1; i < count; ++i) {
                Byte expected = padding == Padding::PKCS7 ? static_cast<Byte>(count) : 0;
                if (data[data.size() - 1 - i] != expected)
                    throw std::runtime_error("invalid padding or wrong key");
            }
        }
        data.resize(data.size() - count);
    }
    Bytes run(Bytes data, bool decrypting) const {
        int n = cipher.blockSize();
        if (!decrypting) addPadding(data);
        if (data.size() % n || (decrypting && data.empty() && padding != Padding::Zeros))
            throw std::runtime_error("invalid ciphertext length");
        Bytes result, state = iv;
        result.reserve(data.size());
        for (size_t pos = 0; pos < data.size(); pos += n) {
            Bytes block(data.begin() + pos, data.begin() + pos + n);
            Bytes output;
            switch (mode) {
                case Mode::ECB:
                    output = decrypting ? cipher.decrypt(block) : cipher.encrypt(block);
                    break;
                case Mode::CBC:
                    if (decrypting) {
                        output = xorBytes(cipher.decrypt(block), state);
                        state = block;
                    } else {
                        output = cipher.encrypt(xorBytes(block, state));
                        state = output;
                    }
                    break;
                case Mode::PCBC:
                    if (decrypting) {
                        output = xorBytes(cipher.decrypt(block), state);
                        state = xorBytes(block, output);
                    } else {
                        output = cipher.encrypt(xorBytes(block, state));
                        state = xorBytes(block, output);
                    }
                    break;
                case Mode::CFB:
                    output = xorBytes(block, cipher.encrypt(state));
                    state = decrypting ? block : output;
                    break;
                case Mode::OFB:
                    state = cipher.encrypt(state);
                    output = xorBytes(block, state);
                    break;
                case Mode::CTR:
                case Mode::RandomDelta: {
                    output = xorBytes(block, cipher.encrypt(state));
                    U64 carry = mode == Mode::CTR ? 1 : delta;
                    for (int i = n - 1; i >= 0; --i) {
                        carry += state[i];
                        state[i] = static_cast<Byte>(carry);
                        carry >>= 8;
                    }
                    break;
                }
            }
            result.insert(result.end(), output.begin(), output.end());
        }
        if (decrypting) removePadding(result);
        return result;
    }
public:
    Context(BlockCipher& c, const Bytes& key, Mode m, Padding p, Bytes initial = {},U64 randomDelta = 0x9e3779b97f4a7c15ULL)
        : cipher(c), mode(m), padding(p), iv(initial), delta(randomDelta) { 
        cipher.setKey(key);
        if (iv.empty()) iv.assign(cipher.blockSize(), 0);
        if (static_cast<int>(iv.size()) != cipher.blockSize())
            throw std::runtime_error("IV must have the same size as the block");
    }
    std::future<Bytes> encrypt(Bytes data) const {
        return std::async(std::launch::async, [this, data]() { return run(data, false); });}
    std::future<Bytes> decrypt(Bytes data) const {
        return std::async(std::launch::async, [this, data]() { return run(data, true); });}
    std::future<void> encrypt(const std::string& from, const std::string& to) const {
        return std::async(std::launch::async, [this, from, to]() { writeFile(to, run(readFile(from), false));});}
    std::future<void> decrypt(const std::string& from, const std::string& to) const {
        return std::async(std::launch::async, [this, from, to]() { writeFile(to, run(readFile(from), true));});}
};

Bytes fromHex(const std::string& text) {
    if (text.size() % 2) throw std::runtime_error("hex string must have even length");
    Bytes result;
    for (size_t i = 0; i < text.size(); i += 2) {
        auto digit = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            throw std::runtime_error("invalid hex digit");
        };
        result.push_back(static_cast<Byte>(digit(text[i]) * 16 + digit(text[i + 1])));
    }
    return result;
}
void processFile(const std::string& input, const std::string& m, const std::string& algorithm, const std::string& p) {
    Mode mode;
    if (m == "ECB") mode = Mode::ECB;
    else if (m == "CBC") mode = Mode::CBC;
    else if (m == "PCBC") mode = Mode::PCBC;
    else if (m == "CFB") mode = Mode::CFB;
    else if (m == "OFB") mode = Mode::OFB;
    else if (m == "CTR") mode = Mode::CTR;
    else if (m == "RandomDelta") mode = Mode::RandomDelta;
    else throw std::runtime_error("unknown mode");
    Padding padding;
    if (p == "Zeros") padding = Padding::Zeros;
    else if (p == "ANSI" || p == "ANSI_X923") padding = Padding::ANSI_X923;
    else if (p == "PKCS7") padding = Padding::PKCS7;
    else if (p == "ISO" || p == "ISO_10126") padding = Padding::ISO_10126;
    else throw std::runtime_error("unknown padding");
    if (algorithm != "DES" && algorithm != "DEAL") throw std::runtime_error("unknown cipher");
    DES des;
    DEAL deal;
    BlockCipher& cipher = algorithm == "DES" ? static_cast<BlockCipher&>(des) : static_cast<BlockCipher&>(deal);
    Bytes key = fromHex(algorithm == "DES" ? "133457799BBCDFF1" : "00112233445566778899AABBCCDDEEFF");
    Bytes iv(cipher.blockSize());
    for (size_t i = 0; i < iv.size(); ++i) iv[i] = static_cast<Byte>(i + 1);
    Context ctx(cipher, key, mode, padding, iv);
    ctx.encrypt(input, input + ".enc").get();
    ctx.decrypt(input + ".enc", input + ".dec").get();
    std::cout << "Created: " << input << ".enc, " << input << ".dec\n";
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "quantity arguments error\n";
        return 1;
    }
    processFile(argv[1], argv[2], argv[3], argv[4]);
    return 0;
}
