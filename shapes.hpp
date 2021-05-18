#include <cmath>
#include <array>

struct Vec3
{
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x{x}, y{y}, z{z} {}
    float normSq() const
    {
        return dot(*this);
    }
    float norm() const
    {
        return sqrt(normSq());
    }
    Vec3 operator+(const Vec3 &rhs) const
    {
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }
    Vec3 operator-(const Vec3 &rhs) const
    {
        return *this + (-rhs);
    }
    Vec3 operator-() const
    {
        return {-x, -y, -z};
    }
    Vec3 operator+(float scalar) const
    {
        return {x + scalar, y + scalar, z + scalar};
    }
    Vec3 operator-(float scalar) const
    {
        return *this + (-scalar);
    }
    Vec3 operator*(float scalar) const
    {
        return {x * scalar, y * scalar, z * scalar};
    }
    Vec3 operator*(const Vec3 &rhs) const
    {
        return {
            x * rhs.x,
            y * rhs.y,
            z * rhs.z};
    }
    Vec3 operator/(float scalar) const
    {
        return *this * (1.f / scalar);
    }
    Vec3 normalized() const
    {
        float len = norm();
        if (len == 0)
        {
            return *this;
        }
        else
        {
            return *this / len;
        }
    }
    float dot(const Vec3 &rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }
    Vec3 cross(const Vec3 &rhs) const
    {
        return {
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x};
    }
    static Vec3 zeros()
    {
        return {0, 0, 0};
    }
    static Vec3 ones()
    {
        return {1, 1, 1};
    }

    Vec3 projection_onto(const Vec3 &rhs) const
    {
        return rhs * (dot(rhs) / rhs.normSq());
    }
    Vec3 projection_onto_plane(const Vec3 &normal_vector) const
    {
        return *this - projection_onto(normal_vector);
    }
    bool isCoolinearWith(const Vec3 &rhs) const
    {
        return cross(rhs).normSq() == 0;
    }
    bool isPerpendicularWith(const Vec3 &rhs) const
    {
        return dot(rhs) == 0;
    }
    Vec3 resized(float new_size) const
    {
        return normalized() * new_size;
    }
};

struct Sphere
{
    Vec3 center;
    float radius;
};

struct Aabb
{
    Vec3 min;
    Vec3 max;
};

struct Tetrahedron
{
    std::array<Vec3, 4> points;
};

struct Capsule
{
    std::array<Vec3, 2> points;
    float radius;
};

template <typename Shape>
Vec3 support(const Vec3 &v, const Shape &s);

template <>
Vec3 support<Sphere>(const Vec3 &v, const Sphere &s)
{
    return s.center + v.resized(s.radius);
}

template <>
Vec3 support<Aabb>(const Vec3 &v, const Aabb &s)
{
    const Vec3 main_diagonal = s.max - s.min;
    const Vec3 mid_point = main_diagonal * 0.5 + s.min;

    std::array<Vec3, 8> corners;
    int idx = 0;
    for (int z : {0, 1})
    {
        for (int y : {0, 1})
        {
            for (int x : {0, 1})
            {
                corners[idx++] = s.min + (main_diagonal * Vec3{x, y, z}) - mid_point;
            }
        }
    }
    std::sort(corners.begin(), corners.end(), [](const Vec3 &lhs, const Vec3 &rhs) {
        if (lhs.z * rhs.z < 0)
        {
            return lhs.z > rhs.z;
        }
        if (lhs.y * rhs.y < 0)
        {
            return lhs.y > rhs.y;
        }
        return lhs.x > rhs.x;
    });
    auto beg = corners.begin();
    auto end = corners.end();
    if (v.z > 0)
    {
        end = beg + (end - beg) / 2;
    }
    else
    {
        beg = beg + (end - beg) / 2;
    }

    if (v.y > 0)
    {
        end = beg + (end - beg) / 2;
    }
    else
    {
        beg = beg + (end - beg) / 2;
    }

    if (v.x > 0)
    {
        end = beg + (end - beg) / 2;
    }
    else
    {
        beg = beg + (end - beg) / 2;
    }
    return *beg + mid_point;
}

template <>
Vec3 support<Tetrahedron>(const Vec3 &v, const Tetrahedron &s)
{
    const auto norm_v = v.normalized();
    std::array<Vec3, 4> normals;
    for (size_t i = 0; i < 4; i++)
    {
        const Vec3 arm1 = s.points[(i + 1) % 4] - s.points[(i + 2) % 4];
        const Vec3 arm2 = s.points[(i + 1) % 4] - s.points[(i + 3) % 4];
        const Vec3 normal = arm1.cross(arm2);
        normals[i] = normal;
    }

    std::array<float, 4> cosines;
    std::transform(normals.begin(), normals.end(), cosines.begin(), [&norm_v](const Vec3 &vec) {
        return norm_v.dot(vec.normalized());
    });
    auto best_abs_cand = std::min_element(cosines.begin(), cosines.end(), [](const auto &lhs, const auto &rhs) {
        return abs(lhs) < abs(rhs);
    });
    const int i_best = best_abs_cand - cosines.begin();
    const auto &normal = normals[i_best];
    if (*best_abs_cand > 0)
    {
        return s.points[i_best];
    }
    if (normal.isCoolinearWith(v))
    {
        const auto &diff_point = s.points[(i_best + 1) % 4];
        return (diff_point - s.points[i_best]).projection_onto_plane(normal) + diff_point;
    }
    std::array<float, 3> dists_sq_from_plane;
    for (size_t i = 0; i < 3; i++)
    {
        dists_sq_from_plane[i] = (s.points[i_best] - s.points[(i_best + 1) % 4]).projection_onto(normal).normSq();
    }
    auto it = std::max_element(dists_sq_from_plane.begin(), dists_sq_from_plane.end());
    return s.points[(i_best + (it - dists_sq_from_plane.begin())) % 4];
}

template <>
Vec3 support<Capsule>(const Vec3 &v, const Capsule &s)
{
    const auto altitude = s.points[0] - s.points[1];
    const auto mid_point = v.dot(altitude) < 0 ? s.points[1] : s.points[0];
    if (v.isCoolinearWith(altitude))
    {
        return mid_point;
    }
    const auto radius = v.projection_onto_plane(altitude).resized(s.radius);
    return mid_point + radius;
}