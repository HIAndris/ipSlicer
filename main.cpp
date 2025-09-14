#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

void nextPowerU32(uint32_t& n) {
    n--;
    for (uint8_t p = 1; p <= 16; p <<= 1) {
        n |= n >> p;
    }
    n++;
}

std::string ipToString(const uint32_t& ip) {
    std::string raw_ip;
    for (int8_t i = 3; i >= 0; i--) {
        raw_ip += std::to_string(ip >> (8 * i) & 0xFF) + (i > 0 ? "." : "");
    }
    return raw_ip;
}

int ipToInt(const std::string& raw_ip, uint32_t& ip) {
    std::stringstream ss(raw_ip);
    std::string part;
    ip = 0;


    for (uint8_t i = 0; i < 4; i++) {
        std::getline(ss, part, '.');
        if (part.empty() || part.length() > 3) {
            return 1;
        }
        uint8_t octet;
        try {
            octet = std::stoi(part);
        }
        catch (const std::invalid_argument&) {
            return 2;
        }
        catch (const std::out_of_range&) {
            return 3;
        }

        ip = (ip << 8) | (octet & 0xFF);
    }
    return 0;
}

int maskToInt(const std::string& raw_mask, uint32_t& mask) {
    uint8_t mask_n;
    try {
        mask_n = std::stoi(raw_mask);
    }
    catch (const std::invalid_argument&) {
        return 2;
    }
    catch (const std::out_of_range&) {
        return 3;
    }

    if (mask_n > 32) {
        return 1;
    }

    mask = 0xFFFFFFFF << (32 - mask_n);
    return 0;
}

/*
typedef enum {
    FLSM,
    VLSM
} patterns_t;

int patternToInt(std::string raw_pattern, patterns_t& pattern) {
    for (char &c : raw_pattern) {
        c = static_cast<char>(tolower(c));
    }
    if (raw_pattern == "FLSM") {pattern = FLSM; return 0;}
    if (raw_pattern == "VLSM") {pattern = VLSM; return 0;}
    return 1;
}

int addSubnetFlsm(const std::string& subnet, std::vector<uint32_t>& subnets, uint32_t& subnets_max, const uint32_t& host_range) {
    uint32_t subnet_n;
    try {
        subnet_n = std::stoi(subnet);
    }
    catch (const std::invalid_argument&) {
        return 2;
    }
    catch (const std::out_of_range&) {
        return 3;
    }

    subnet_n += 2;
    nextPowerU32(subnet_n);
    uint32_t subnets_max_temp;
    if (subnets_max < subnet_n) {
        subnets_max_temp = subnet_n;
    } else {
        subnets_max_temp = subnets_max;
    }

    if (subnets_max_temp * subnets.size() > host_range) {
        return 1;
    }

    subnets_max = subnets_max_temp;
    subnets.push_back(subnet_n);
    return 0;
}
*/

int addSubnetVlsm(const std::string& subnet, std::vector<uint32_t>& subnets, uint32_t& subnets_sum, const uint32_t& host_range) {
    uint32_t subnet_n;
    try {
        subnet_n = std::stoi(subnet);
    }
    catch (const std::invalid_argument&) {
        return 2;
    }
    catch (const std::out_of_range&) {
        return 3;
    }

    subnet_n += 2;
    nextPowerU32(subnet_n);

    if (subnet_n + subnets_sum > host_range) {
        return 1;
    }

    subnets_sum += subnet_n;
    subnets.push_back(subnet_n);
    return 0;
}

int input(uint32_t& ip, uint32_t& mask, /*patterns_t& pattern,*/ std::vector<uint32_t>& subnets) {
    std::string raw_ip;
    std::string raw_mask;
    std::string raw_pattern;
    std::string subnet;
    uint32_t subnets_sum = 0;
    uint8_t retries = 0;

    while (true) {
        if (retries > 20) {
            return 1;
        }
        retries++;

        std::cout << "\nIPv4: ";
        std::getline(std::cin, raw_ip);
        if (ipToInt(raw_ip, ip)) {
            std::cout << "Invalid IP address! (0.0.0.0 - 255.255.255.255)" << std::endl;
            continue;
        }

        std::cout << "Mask: /";
        std::getline(std::cin, raw_mask);
        if (maskToInt(raw_mask, mask)) {
            std::cout << "Invalid mask! (/0 - /32)" << std::endl;
            continue;
        }
        const uint32_t host_range = ~mask;

        /*
        std::cout << "Pattern: " << std::endl;
        std::getline(std::cin, raw_pattern);
        if (!patternToInt(raw_pattern, pattern)) {
            std::cout << "Invalid pattern! (FLSM/VLSM)" << std::endl;
            continue;
        }
        */

        std::cout << "\nSubnets:\n(end with empty input)\n1.:\nNumber of hosts: ";
        std::getline(std::cin, subnet);
        while (!subnet.empty()) {
            switch (addSubnetVlsm(subnet, subnets, subnets_sum, host_range)) {
                case 0:
                    break;
                case 1:
                    std::cout << "Subnet too big!" << std::endl;
                    break;
                default:
                    std::cout << "Invalid subnet! (host count)" << std::endl;
            }
            std::cout << subnets.size() + 1 << ".:\n" << "Number of hosts: ";
            std::getline(std::cin, subnet);
        }

        return 0;
    }
}

typedef enum {
    ID,
    HOST_N,
    NETWORK_ADDRESS,
    BROADCAST_ADDRESS,
    FIRST_USABLE_ADDRESS,
    LAST_USABLE_ADDRESS,
    MASK
} subnet_element_t;

typedef std::map<subnet_element_t, uint32_t> subnet_t;

void calculateSubnetsVlsm(uint32_t network_address, const std::vector<uint32_t>& subnets, std::vector<subnet_t>& calculated_subnets) {
    calculated_subnets.clear();
    for (uint32_t i = 0; i < subnets.size(); i++) {
        calculated_subnets.emplace_back();
        calculated_subnets[i][ID] = i + 1;
        calculated_subnets[i][HOST_N] = subnets[i];
    }

    constexpr struct {
        bool operator()(subnet_t a, subnet_t b) const { return a[HOST_N] > b[HOST_N]; }
    } sortByHostN;
    std::sort(calculated_subnets.begin(), calculated_subnets.end(), sortByHostN);

    for (subnet_t& calculated_subnet: calculated_subnets) {
        calculated_subnet[NETWORK_ADDRESS] = network_address;
        if (calculated_subnet[HOST_N] > 2) {
            calculated_subnet[FIRST_USABLE_ADDRESS] = network_address + 1;
            network_address += calculated_subnet[HOST_N];
            calculated_subnet[LAST_USABLE_ADDRESS] = network_address - 2;
        } else {
            calculated_subnet[FIRST_USABLE_ADDRESS] = 0;
            network_address += calculated_subnet[HOST_N];
            calculated_subnet[LAST_USABLE_ADDRESS] = 0;
        }
        calculated_subnet[BROADCAST_ADDRESS] = network_address - 1;
        calculated_subnet[MASK] = ~(calculated_subnet[HOST_N] - 1);
    }
}

void output(const uint32_t& network_address, const uint32_t& mask, const std::vector<subnet_t>& calculated_subnets) {
    std::cout << "\nNetwork:\n" << ipToString(network_address) << " " << ipToString(mask) << "\n" << std::endl;
    for (const subnet_t& calculated_subnet : calculated_subnets) {
        std::cout << calculated_subnet.at(ID) << ".:" << "\n"
        << "address:   " << ipToString(calculated_subnet.at(NETWORK_ADDRESS)) << " " << ipToString(calculated_subnet.at(MASK)) << "\n"
        << "first:     " << ipToString(calculated_subnet.at(FIRST_USABLE_ADDRESS)) << "\n"
        << "last:      " << ipToString(calculated_subnet.at(LAST_USABLE_ADDRESS)) << "\n"
        << "broadcast: " << ipToString(calculated_subnet.at(BROADCAST_ADDRESS)) << "\n"
        << "(fits " << calculated_subnet.at(HOST_N) - 2 << " hosts)\n" << std::endl;
    }
    std::cout << "By HIAndris.\nDistribute freely without modifying.\nFound a bug? Message me on Discord! (hiandris)" << std::endl;
}

int main() {
    uint32_t ip;
    uint32_t mask;
    std::vector<uint32_t> subnets;
    std::vector<subnet_t> calculated_subnets;

    if (input(ip, mask, subnets)) {
        std::cout << "Stop wasting my time!" << std::endl;
        return 1;
    }

    const uint32_t network_address = ip & mask;
    calculateSubnetsVlsm(network_address, subnets, calculated_subnets);

    output(network_address, mask, calculated_subnets);

    return 0;
}