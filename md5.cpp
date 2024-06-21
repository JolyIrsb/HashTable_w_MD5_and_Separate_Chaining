
#include <iostream>
#include <string>
#include <cstdint>
#include <array>

using namespace std;

// Левый циклический сдвиг
uint32_t leftRotate(uint32_t x, uint32_t c) {
    return (x << c) | (x >> (32 - c));
}


// Функция MD5
string md5(string message) {
    // Константы для циклического сдвига влево 
    constexpr uint32_t s[64] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7,
        12, 17, 22, 5, 9 , 14, 20, 5, 9 , 14, 20, 5,
        9 , 14, 20, 5, 9 , 14, 20, 4, 11, 16, 23, 4,
        11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6 ,10 ,15 ,21 ,6 ,10 ,15 ,21 ,6 ,10 ,15 ,21 ,
        6 ,10 ,15 ,21
    };
    // Константы, каждый элемент определяется выражением int(2^32 * |sin n|)
    constexpr uint32_t K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    // Инициализация переменных
    uint32_t a0 = 0x67452301U; // A
    uint32_t b0 = 0xefcdab89U; // B
    uint32_t c0 = 0x98badcfeU; // C
    uint32_t d0 = 0x10325476U; // D

    // Добавление бита "1" к сообщению
    message += char(0x80);

    // Добавление битов "0" до длины сообщения mod512 ==448
    while ((message.size() * 8) % 512 != 448) {
        message += char(0);
    }

    // Добавление длины сообщения в битах в конец сообщения
    uint64_t messageSizeBits = message.size() * 8;
    for (int i = 0; i < 8; i++) {
        message += char((messageSizeBits >> (i * 8)) & 0xFF);
    }

    // Обработка каждого блока по 512 бит (64 байта)
    for (size_t i = 0; i < message.size(); i += 64) {
        array<uint32_t, 16> M; // Массив слов M[0..15]
        for (int j = 0; j < 16; j++) { //Разбитие на 16 блоков
            M[j] = (uint8_t(message[i + j * 4]) << 0)
                | (uint8_t(message[i + j * 4 + 1]) << 8)
                | (uint8_t(message[i + j * 4 + 2]) << 16)
                | (uint8_t(message[i + j * 4 + 3]) << 24);
        }

        uint32_t A = a0;
        uint32_t B = b0;
        uint32_t C = c0;
        uint32_t D = d0;

        for (int j = 0; j < 64; j++) {
            uint32_t F, g;

            if (j >= 0 && j <= 15) {
                F = (B & C) | ((~B) & D);
                g = j;
            }
            else if (j >= 16 && j <= 31) {
                F = (D & B) | ((~D) & C);
                g = (5 * j + 1) % 16;
            }
            else if (j >= 32 && j <= 47) {
                F = B ^ C ^ D;
                g = (3 * j + 5) % 16;
            }
            else if (j >= 48 && j <= 63) {
                F = C ^ (B | (~D));
                g = (7 * j) % 16;
            }

            F += A + K[j] + M[g];
            A = D;
            D = C;
            C = B;
            B += leftRotate(F, s[j]);
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    // Формирование хеша из четырех 32-битных слов
    string hash;
    for (int i = 0; i < 4; i++) {
        hash += char((a0 >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < 4; i++) {
        hash += char((b0 >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < 4; i++) {
        hash += char((c0 >> (i * 8)) & 0xFF);
    }
    for (int i = 0; i < 4; i++) {
        hash += char((d0 >> (i * 8)) & 0xFF);
    }

    return hash;
}

// Функция для вывода хеша в шестнадцатеричном формате
string hexString(string hash) {
    string hex;
    for (char c : hash) {
        uint8_t byte = uint8_t(c);
        hex += "0123456789abcdef"[byte >> 4];
        hex += "0123456789abcdef"[byte & 0xF];
    }
    return hex;
}
