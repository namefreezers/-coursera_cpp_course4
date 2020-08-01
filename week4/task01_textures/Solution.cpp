#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard"

#include "Common.h"

using namespace std;

bool operator<(const Point &p, const Size &sz) {
    return p.x < sz.width && p.y < sz.height;
}

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`

class FigureShape : public IShape {
public:
    void SetPosition(Point p) override {
        pos = p;
    }

    Point GetPosition() const override {
        return pos;
    }

    void SetSize(Size s) override {
        sz = s;
    }

    Size GetSize() const override {
        return sz;
    }

    void SetTexture(shared_ptr<ITexture> t) override {
        texture = t;
    }

    ITexture *GetTexture() const override {
        return texture.get();
    }

    void Draw(Image &im) const override {
        for (int y = 0; y < min<size_t>(sz.height, im.size() - pos.y); y++) {
            for (int x = 0; x < min<size_t>(sz.width, im[0].size() - pos.x); x++) {

                if (!IsFigureContainsPoint(x, y)) {
                    continue;
                }

                if (texture && Point{x, y} < texture->GetSize()) {
                    im[y + pos.y][x + pos.x] = texture->GetImage()[y][x];
                } else {  // not texture or out of texture
                    im[y + pos.y][x + pos.x] = '.';
                }
            }
        }
    }

private:
    virtual bool IsFigureContainsPoint(int x, int y) const = 0;

protected:
    Point pos;
    Size sz;
    shared_ptr<ITexture> texture;
};

class RectShape : public FigureShape {
    unique_ptr<IShape> Clone() const override {
        return make_unique<RectShape>(*this);
    }

private:
    bool IsFigureContainsPoint(int x, int y) const {
        return Point{x, y} < sz;
    }
};

class EllipseShape : public FigureShape {
    unique_ptr<IShape> Clone() const override {
        return make_unique<EllipseShape>(*this);
    }

private:
    bool IsFigureContainsPoint(int x, int y) const {
        return IsPointInEllipse({x,y}, sz);
    }
};

// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    switch (shape_type) {
        case ShapeType::Rectangle:
            return make_unique<RectShape>();
        case ShapeType::Ellipse:
            return make_unique<EllipseShape>();
        default:
            return nullptr;
    }
}


#pragma clang diagnostic pop