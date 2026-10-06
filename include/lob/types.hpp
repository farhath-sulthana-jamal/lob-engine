#pragma once

class Price {
    private:
        unsigned long value;
    
    public:
        Price(unsigned long v) : value(v) {}
        unsigned long getValue() const {
            return value;
        }
    
};

class Qty {
    private:
        unsigned int value;
    
    public:
        Qty(unsigned int v) : value(v) {}
        unsigned int getValue() const { 
            return value;
        }
};