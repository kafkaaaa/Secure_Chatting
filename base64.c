#include "base64.h"

const char base64code_ascii[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const unsigned char asciicode_base64code[256] =
    {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        // 0x2b +, 0x2f /
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 62, 0xff, 0xff, 0xff, 63,
        // 0x30 : 0,1,2,...
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        // 0x40: ＠,A,B,C,...
        0xff, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        // 0x50: P,Q,R,S,...
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0xff, 0xff, 0xff, 0xff, 0xff,
        // 0x60 : ｀,a,b,c,...
        0xff, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        // 0x70 : p,q,r,s,...
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int text_to_base64_size(const char *text)
{
    int len = strlen(text);
    return ((len + 2) / 3 * 4);
}

int base64_encoder(char *in, int in_size, char *out, int out_size)
{
    int i = 0;
    int o = 0;

    if (!in || !out)
        return 0;
    if (text_to_base64_size(in) > out_size)
        return 0;

    while (i < in_size - 2 && o + 4 <= out_size)
    {
        out[o++] = base64code_ascii[(in[i] >> 2) & 0x3F];
        out[o++] = base64code_ascii[((in[i] & 0x3) << 4) | ((int)(in[i + 1] & 0xF0) >> 4)];
        out[o++] = base64code_ascii[((in[i + 1] & 0xF) << 2) | ((int)(in[i + 2] & 0xC0) >> 6)];
        out[o++] = base64code_ascii[in[i + 2] & 0x3F];
        i += 3;
    }

    if (i < in_size)
    {
        out[o++] = base64code_ascii[(in[i] >> 2) & 0x3F];
        if (i == (in_size - 1))
        {
            out[o++] = base64code_ascii[((in[i] & 0x3) << 4)];
            out[o++] = '='; // padding
        }
        else
        {
            out[o++] = base64code_ascii[((in[i] & 0x3) << 4) | ((int)(in[i + 1] & 0xF0) >> 4)];
            out[o++] = base64code_ascii[((in[i + 1] & 0xF) << 2)];
        }

        out[o++] = '='; // padding
    }

    return o;
}

int base64code_size(const char *code, int len)
{
    int real_code_len = 0;
    if (!code)
        return 0;

    while (real_code_len < len && asciicode_base64code[code[real_code_len]] != 0xff)
    {
        real_code_len++;
    }

    return real_code_len;
}

int base64_to_text_size(const char *code, int len)
{
    return ((base64code_size(code, len) + 3) / 4) * 3;
}

int base64_decoder(const char *in, int in_size, char *out, int out_size)
{
    int i = 0;
    int o = 0;
    int real_code_len = 0;

    if (!in || !out)
        return 0;
    if (out_size < base64_to_text_size(in, in_size))
        return 0;

    real_code_len = base64code_size(in, in_size);

    while (i + 4 < real_code_len)
    {
        out[o++] = (char)(asciicode_base64code[in[i]] << 2 | asciicode_base64code[in[i + 1]] >> 4);
        out[o++] = (char)(asciicode_base64code[in[i + 1]] << 4 | asciicode_base64code[in[i + 2]] >> 2);
        out[o++] = (char)(asciicode_base64code[in[i + 2]] << 6 | asciicode_base64code[in[i + 3]]);
        i += 4;
    }

    if (real_code_len - i > 1)
    {
        out[o++] = (char)(asciicode_base64code[in[i]] << 2 | asciicode_base64code[in[i + 1]] >> 4);
    }

    if (real_code_len - i > 2)
    {
        out[o++] = (char)(asciicode_base64code[in[i + 1]] << 4 | asciicode_base64code[in[i + 2]] >> 2);
    }

    if (real_code_len - i > 3)
    {
        out[o++] = (char)(asciicode_base64code[in[i + 2]] << 6 | asciicode_base64code[in[i + 3]]);
    }

    return o;
}

