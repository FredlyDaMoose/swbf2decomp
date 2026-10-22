#pragma once

class RedColor
{
public:

    // FUNCTION: SWBF2 0x004108a0
    RedColor() {}

    // FUNCTION: SWBF2 0x0040f750
    RedColor(const RedColor& c) : b(c.b), g(c.g), r(c.r), a(c.a) {}
    RedColor(unsigned char aR, unsigned char aG, unsigned char aB, unsigned char aA)
        : b(aB), g(aG), r(aR), a(aA) {}

    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;

    unsigned int GetARGB() const;

    static RedColor WHITE;
    static RedColor LTGREY;
    static RedColor MDGREY;
    static RedColor DKGREY;
    static RedColor BLACK;
    static RedColor RED;
    static RedColor GREEN;
    static RedColor BLUE;
    static RedColor CYAN;
    static RedColor MAGENTA;
    static RedColor YELLOW;

    // FUNCTION: SWBF2 0x00525ee0
    const RedColor operator*(const RedColor& c) const
    {
        RedColor result = *this;
        result *= c;
        return result;
    }

    // FUNCTION: SWBF2 0x00492000
    const RedColor& operator*=(const RedColor& c)
    {
        r = (unsigned char)(int)(c.r * (r * (1.0f / 255.0f)) + 0.5f);
        g = (unsigned char)(int)(g * (1.0f / 255.0f) * c.g + 0.5f);
        b = (unsigned char)(int)(b * (1.0f / 255.0f) * c.b + 0.5f);
        a = (unsigned char)(int)(c.a * (a * (1.0f / 255.0f)) + 0.5f);
        return *this;
    }
};

 
