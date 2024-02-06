#include <bit>
#include <bitset>
#include <format>
#include <iostream>
#include <string>

#include "memory_bus.hpp"
#include "uartlite.hpp"
#include "ram.hpp"
#include "rv_core.hpp"
#include "rv_systembus.hpp"
#include "rv_clint.hpp"
#include "rv_plic.hpp"
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <signal.h>

bool riscv_test;

static_assert(CHAR_BIT == 8);
static_assert(std::endian::native == std::endian::little);

static unsigned char parse_hex(char val) {
    if (val == '0') return 0x0;
    if (val == '1') return 0x1;
    if (val == '2') return 0x2;
    if (val == '3') return 0x3;
    if (val == '4') return 0x4;
    if (val == '5') return 0x5;
    if (val == '6') return 0x6;
    if (val == '7') return 0x7;
    if (val == '8') return 0x8;
    if (val == '9') return 0x9;
    if (val == 'a' || val == 'A') return 0xa;
    if (val == 'b' || val == 'B') return 0xb;
    if (val == 'c' || val == 'C') return 0xc;
    if (val == 'd' || val == 'D') return 0xd;
    if (val == 'e' || val == 'E') return 0xe;
    if (val == 'f' || val == 'F') return 0xf;
    assert(false);
}

struct qtest : public mmio_dev {
public:
    bool do_read(uint64_t start_addr, uint64_t size, unsigned char* buffer) override {
        assert(size < SIZE_MAX);
        std::cout << std::format("read {:#x} {}\n", start_addr, size);
        std::string response;
        std::getline(std::cin, response);
        size_t pos = response.find(' ');
        std::string keyword = response.substr(0, pos);
        if (keyword != "OK") {
            throw;
        }
        assert(response.substr(pos, 3) == " 0x");
        pos += 3;
        assert(response.size() - pos == size * 2);
        for (size_t i = 0; i != size; i ++) {
            buffer[i] = parse_hex(response[pos + i * 2]) * 16 + parse_hex(response[pos + i * 2 + 1]);
        }
        return true;
    }

    bool do_write(uint64_t start_addr, uint64_t size, const unsigned char* buffer) override {
        assert(size < SIZE_MAX);
        std::string data;
        data.reserve(size * 2);

        static const char hex_chars[] = "0123456789abcdef";
        for (size_t i = 0; i != size; i ++) {
            data.push_back(hex_chars[buffer[i] >> 4]);
            data.push_back(hex_chars[buffer[i] & 0xf]);
        }
        std::cout << std::format("write {:#x} {} 0x{}\n", start_addr, size, data);
        std::cout.flush();
        std::string response;
        std::getline(std::cin, response);
        if (response != "OK") throw;
        return true;
    }
};


int main(int argc, const char* argv[]) {

    rv_systembus system_bus;

    // uartlite uart;
    // rv_clint<2> clint;
    // rv_plic <4,4> plic;
    // ram dram(4096l*1024l*1024l,load_path);
    // assert(system_bus.add_dev(0, ((uint64_t) 1) << 63,&clint));
    // assert(system_bus.add_dev(0xc000000,0x4000000,&plic));
    // assert(system_bus.add_dev(0x60100000,1024*1024,&uart));
    // assert(system_bus.add_dev(0x80000000,2048l*1024l*1024l,&dram));

    qtest q;
    system_bus.add_dev(0, ((uint64_t) 1) << 63, &q);

    rv_core rv_0(system_bus,0);
    // rv_core rv_1(system_bus,1);

    rv_0.jump(0x1000);
    // rv_1.jump(0x80000000);
    // char uart_history[8] = {0};
    // int uart_history_idx = 0;
    while (1) {
        // clint.tick();
        // plic.update_ext(1,uart.irq());
        // rv_0.step(plic.get_int(0),clint.m_s_irq(0),clint.m_t_irq(0),plic.get_int(1));
        // rv_1.step(plic.get_int(2),clint.m_s_irq(1),clint.m_t_irq(1),plic.get_int(3));
        rv_0.step(0, 0, 0, 0);
        std::cerr << std::format("pc = {:#x}\n", rv_0.getPC());
        assert(rv_0.getPC());
    }
    return 0;
}
