#ifndef _SEG_HPP_
#define _SEG_HPP_


////////////
// base class: common info for 

struct Seg {
    //{entry, exit} {point, tangent}
    float x1, y1, theta1;
    float x2, y2, theta2;
    float length;
    explicit Seg(float x1, float y1, float x2, float y2)
                :x1(x1), y1(y1), x2(x2), y2(y2) {}
    virtual void exec(float v1, float v2, float v3) = 0;
};


////////////
// polymorphic sub-classes

struct MoveSeg : Seg {
    explicit MoveSeg(float x1, float y1, float x2, float y2);
    virtual void exec(float v1, float v2, float v3);
};

struct LineSeg : Seg {
    explicit LineSeg(float x1, float y1, float x2, float y2);
    virtual void exec(float v1, float v2, float v3);
};


#endif//_SEG_HPP_
