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

    inline bool solid() const { return length >= 0; } //if segment should be drawn
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

struct ArcSeg : Seg {
    float r1, r2, phi;
    bool is_large, is_sweep;

    float cx, cy, a1, a2; //NOTE: a1/a2 => radial ; theta1/theta2 => tangent

    explicit ArcSeg(float x1, float y1, float x2, float y2,
                    float r1, float r2, float phi,
                    bool is_large, bool is_sweep);
    virtual void exec(float v1, float v2, float v3);
};


#endif//_SEG_HPP_
