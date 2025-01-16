#pragma once

#include "Root/Root.h"

#include "PerlinNoise.hpp"

#include "EASTL/optional.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifdef PlaySound
#undef PlaySound
#endif

namespace Orin::Overkill::Utils
{
    namespace Vector
    {            
        template<typename T>
        inline Math::Vector2 xy(T v)
        {
            return { v.x, v.y };
        }

        template<typename T>
        inline Math::Vector2 xz(T v)
        {
            return { v.x, v.z };
        }

        inline Math::Vector3 Vyz(Math::Vector3 vec, float value)
        {
            return { value, vec.y, vec.z };
        }

        inline Math::Vector3 xVz(Math::Vector3 vec, float value)
        {
            return { vec.x, value, vec.z };
        }

        inline Math::Vector3 xyV(Math::Vector3 vec, float value)
        {
            return { vec.x, vec.y, value };
        }

        template<typename T>
        inline Math::Vector3 Oyz(T vec)
        {
            return { 0.f, vec.y, vec.z };
        }

        template<typename T>
        inline Math::Vector3 xOz(T vec)
        {
            return { vec.x, 0.f, vec.z };
        }

        template<typename T>
        inline Math::Vector3 xyO(T vec)
        {
            return { vec.x, vec.y, 0.f };
        }

        template<typename T>
        inline T Normalize(T v)
        {
            T nV(v);
            nV.Normalize();
            return nV;
        }

        template<typename T>
        inline eastl::tuple<T, float> Normalize2(T v)
        {
            T nV(v);
            const float length = nV.Normalize();
            return eastl::make_tuple(nV, length);
        }

        inline Math::Vector2 Rotate90(Math::Vector2 vec)
        {
            return {-vec.y, vec.x};
        }

        inline Math::Vector3 Rotate90(Math::Vector3 vec)
        {
            return xyO(Rotate90(xy(vec)));
        }

        inline Math::Vector2 Rotate(Math::Vector2 vec, float angle)
        {
			Math::Matrix rotZ;
			rotZ.RotateZ(angle);
			return xy(rotZ.MulNormal(xyO(vec)));
        }

        inline Math::Vector3 Rotate(Math::Vector3 vec, float angle)
        {
			Math::Matrix rotZ;
			rotZ.RotateZ(angle);
			return rotZ.MulNormal(vec);
        }

        template<typename T>
        inline T Reflect(T d, T n)
        {
            return d - 2.f * d.Dot(n) * n;
        }
    }

    inline float SafeInvert(float val) { return fabs(val) > 1.e-5f ? (1.f / val) : 0.f; }
    inline double SafeInvert(double val) { return abs(val) > 1.e-5 ? (1. / val) : 0.; }

    inline float Saturate(float val) { return Math::Clamp(val, 0.f, 1.f); }
    inline float Square(float val) { return val * val; }
    inline float Fract(float val) { return val - ::floorf(val); }

    inline uint32_t MakeColor(float brightness)
    {
        const uint32_t ib = uint32_t(Saturate(brightness) * 255.f + 0.5f);
        return 0xFF000000 | ib | (ib << 8u) | (ib << 16u);
    }

    inline uint32_t MakeColor(float brightness, float alpha)
    {
        const uint32_t ib = uint32_t(Saturate(brightness) * 255.f + 0.5f);
        const uint32_t ia = uint32_t(Saturate(alpha) * 255.f + 0.5f);
        return ib | (ib << 8u) | (ib << 16u) | (ia << 24u);
    }

    inline float Smoothstep(float edge0, float edge1, float x)
    {
        const float t = Saturate((x - edge0) / (edge1 - edge0));
        return t * t * (3.f - 2.f * t);
    }

    template<typename T>
    inline T Lerp(T x, T y, float t) { return x + (y - x) * t; }

    inline Math::Vector2 Clamp(Math::Vector2 v, Math::Vector2 from, Math::Vector2 to)
    {
        return {Math::Clamp(v.x, from.x, to.x), Math::Clamp(v.y, from.y, to.y)};
    }

    inline Math::Vector3 Clamp(Math::Vector3 v, Math::Vector3 from, Math::Vector3 to)
    {
        return {Math::Clamp(v.x, from.x, to.x), Math::Clamp(v.y, from.y, to.y), Math::Clamp(v.z, from.z, to.z)};
    }

    template<typename T>
    inline bool IsNonZero(T a, float eps = 1e-5f)
    {
        return Math::IsNonZero(a.Length2());
    }

    inline float FSel(float x, float a, float b)
    {
        return x >= 0.f ? a : b;
    }

    inline Math::Quaternion DirectionToQuaternion(Math::Vector3 d)
    {
        const float heading  = Math::SafeAtan2(-d.z, d.x) * 0.5f;
        const float attitude = Math::SafeAtan2(d.y, Vector::xz(d).Length()) * 0.5f;
        const float s1 = sinf(heading);
        const float c1 = cosf(heading);
        const float s2 = sinf(attitude);
        const float c2 = cosf(attitude);

        return Math::Quaternion(s1 * s2, s1 * c2, c1 * s2, c1 * c2);
    }

    inline Math::Quaternion Approach(Math::Quaternion from, Math::Quaternion to, float dt, float viscosity)
    {
        if (viscosity < 1e-5f)
            return to;
        Math::Quaternion res;
        res.SLerp(from, to, 1.f - expf(-dt / viscosity));
        return res;
    }

    inline Math::Vector3 GetForward(Math::Quaternion q)
    {
        return Vector::Normalize(Math::Vector3{q.x * q.x + q.w * q.w - .5f,
                                               q.y * q.x + q.z * q.w,
                                               q.z * q.x - q.y * q.w});
    }

    inline Math::Vector3 GetUp(Math::Quaternion q)
    {
        return Vector::Normalize(Math::Vector3{q.x * q.y - q.z * q.w,
                                               q.y * q.y + q.w * q.w - .5f,
                                               q.z * q.y + q.x * q.w});
    }

    template<typename T>
    inline T Approach(T from, T to, float dt, float viscosity)
    {
        if (viscosity < 1e-5f)
            return to;
        return from + (1.f - expf(-dt / viscosity)) * (to - from);
    }

    inline float ToDegrees(float a)
    {
        static const float invRadian = 1.f / Math::Radian;
        return a * invRadian;
    }

    inline float ToRadian(float a)
    {
        return a * Math::Radian;
    }

    // Normalize angle to the range [0; TwoPI)
    inline float NormalizeAngle(float a)
    {
        if (a >= 0.f && a < Math::TwoPI)
            return a;
        return a >= 0.f ? fmodf(a, Math::TwoPI) : Math::TwoPI + fmodf(a, Math::TwoPI);
    }

    // Normalize angle to the range [-PI; PI]
    inline float NormalizeSignedAngle(float a)
    {
        a = fmodf(a, Math::TwoPI);
        return a > Math::PI ? a - Math::TwoPI : (a < -Math::PI ? a + Math::TwoPI : a);
    }

    // [0; TwoPI]
    inline float DistanceAngle(float a, float b)
    {
        return NormalizeAngle(NormalizeAngle(b) - NormalizeAngle(a));
    }

    // [0; TwoPI]
    inline float ClampAngle(float a, float from, float to)
    {
        const float na = NormalizeAngle(a);
        const float d = DistanceAngle(from, to);
        if (d <= 1e-3 || DistanceAngle(from, na) <= d)
        {
            return na;
        }
        return DistanceAngle(na, from) < DistanceAngle(to, na) ? NormalizeAngle(from) : NormalizeAngle(to);
    }

    // [-PI; PI]
    inline float ClampSignedAngle(float angle, float baseAngle, float minDelta, float maxDelta)
    {
        const float delta = NormalizeSignedAngle(baseAngle - angle);
        return angle + std::min(-std::min(minDelta, 0.f) + delta, 0.f) - std::min(std::max(maxDelta, 0.f) - delta, 0.f);
    }

    // [-PI; PI]
    inline float ClampSignedAngle(float angle, float minAngle, float maxAngle)
    {
        const float normMin = NormalizeSignedAngle(minAngle - angle);
        const float normMax = NormalizeSignedAngle(maxAngle - angle);
        return (normMin <= 0.f && normMax >= 0.f) ? angle : (std::abs(normMax) - std::abs(normMin) >= 0.f ?  minAngle : maxAngle);
    }

    // [-PI; PI]
    inline float AdvanceAngle(float angle, float delta)
    {
        return NormalizeSignedAngle(NormalizeAngle(angle) + delta);
    }

    // Normalize angle to the range [0; 360)
    inline float NormalizeAngleDegress(float a)
    {
        return ToDegrees(NormalizeAngle(ToRadian(a)));
    }

    // Normalize angle to the range [-180; 180]
    inline float NormalizeSignedAngleDegress(float a)
    {
        return ToDegrees(NormalizeSignedAngle(ToRadian(a)));
    }

    // [0; 360]
    inline float DistanceAngleDegress(float a, float b)
    {
        return ToDegrees(DistanceAngle(ToRadian(a), ToRadian(b)));
    }

    // [0; 360]
    inline float ClampAngleDegress(float a, float from, float to)
    {
        return ToDegrees(ClampAngle(ToRadian(a), ToRadian(from), ToRadian(to)));
    }

    // [-180; 180]
    inline float ClampSignedAngleDegress(float angle, float baseAngle, float minAngle, float maxAngle)
    {
        return ToDegrees(ClampSignedAngle(ToRadian(angle), ToRadian(baseAngle), ToRadian(minAngle), ToRadian(maxAngle)));
    }

    // [-180; 180]
    inline float ClampSignedAngleDegress(float angle, float minAngle, float maxAngle)
    {
        return ToDegrees(ClampSignedAngle(ToRadian(angle), ToRadian(minAngle), ToRadian(maxAngle)));
    }

    // [-PI; PI]
    inline float AdvanceAngleDegress(float angle, float delta)
    {
        return NormalizeSignedAngleDegress(NormalizeAngleDegress(angle) + delta);
    }

    struct Angle
    {
        struct TRadian{};
        struct TDegrees{};

        // [-PI; PI]
        float radian;

        Angle() : radian(0.f) {};

        template<typename Mode = TRadian>
        explicit Angle(float _value, Mode mode);

        explicit Angle(float _radian, TRadian)   : radian(Utils::NormalizeSignedAngle(_radian)) {}
        explicit Angle(float _degrees, TDegrees) : radian(Utils::NormalizeSignedAngle(Utils::ToRadian(_degrees))) {}

        inline static Angle Radian(float radian)   { return Angle(radian,  TRadian{}); }
        inline static Angle Degrees(float degrees) { return Angle(degrees, TDegrees{}); }

        template <typename T>
        inline static Angle FromDirection(T dir) { return Angle(Math::SafeAtan2(dir.y, dir.x), TRadian{}); }

        inline void operator+=(Angle delta) { radian = Utils::NormalizeSignedAngle(radian + delta.radian); }
        inline void operator-=(Angle delta) { radian = Utils::NormalizeSignedAngle(radian - delta.radian); }

        inline Angle operator+(Angle a) const { return Radian(radian + a.radian); }
        inline Angle operator-(Angle a) const { return Radian(radian - a.radian); }

        inline Angle operator*(float s) const { return Radian(radian * s); }
        inline Angle operator/(float s) const { return Radian(radian / s); }

        inline Angle operator-() const { return Radian(-radian); }

        inline Angle Add(float a, TRadian) const { return Radian(radian + a); }
        inline Angle Add(float a, TDegrees) const { return Radian(radian + Utils::ToRadian(a)); }

        inline bool IsEqual(Angle a, float eps = 16.f * FLT_EPSILON) const
        {
            return Math::AreApproximatelyEqual(Distance(a).radian, 0.f, eps);
        }

        //  1: this > A
        // -1: this < A
        //  0: this == A
        inline int Compare(Angle a) const
        {
            if (IsEqual(a))
                return 0;

            const Angle dist = *this - a;
            return dist.radian > 0.f ? 1 : dist.radian < 0.f ? -1 : 0;
        }

        inline bool operator>(Angle a) const  { return Compare(a) == 1; }
        inline bool operator<(Angle a) const  { return Compare(a) == -1; }
        inline bool operator>=(Angle a) const { return Compare(a) >= 0; }
        inline bool operator<=(Angle a) const { return Compare(a) <= 0; }

        // Shortest distance between angles
        inline Angle Distance(Angle a) const
        {
            const Angle dist = a - *this;
            return Radian(std::abs(dist.radian));
        }

        inline Angle Clamp(Angle from, Angle to) const
        {
            return Radian(Utils::ClampAngle(ToRadian(), from.ToRadian(), to.ToRadian()));
        }

        inline Angle MoveTo(Angle target, float vel) const
        {
            if (IsEqual(target))
                return target;

            // [0; PI]
            const float dist  = Distance(target).radian;
            const float delta = vel * float(target.Compare(*this));
            // Distanse between this and target must always decrease
            return dist - std::abs(delta) > 0.f ? Radian(radian + delta) : target;
        }

        inline Math::Vector2 ToDirection() const
        {
            return {cosf(radian), sinf(radian)};
        }

        // [0; 360]
        inline float ToDegrees() const { return Utils::NormalizeAngleDegress(ToSignedDegrees()); }
        // [0; TwoPI]
        inline float ToRadian() const  { return Utils::NormalizeAngle(radian); }

        // [-180; 180]
        inline float ToSignedDegrees() const { return Utils::ToDegrees(radian); }
        // [-PI; PI]
        inline float ToSignedRadian() const  { return radian; }

        static inline Angle Distance(Angle a, Angle b)
        {
            return a.Distance(b);
        }
    };

    // Squirrel Eiserloh's hash function
    uint64_t UIntNoise1D(uint64_t position);

    struct RndGenerator
    {
        uint64_t seed;
        uint64_t position;

        RndGenerator(uint64_t _seed = 0)
        {
            seed     = _seed == 0 ? UIntNoise1D((uint64_t)time(nullptr)) : _seed;
            position = UIntNoise1D(seed);
        };

        uint64_t NextUInt()
        {
            position = UIntNoise1D(position);
            return position;
        }

        // [0; 1]
        float NextFloat()
        {
            const uint64_t FloatMask = (1 << 24) - 1;
            const float    MaxFloat  = 16777215.f;
            return float(NextUInt() & FloatMask) / MaxFloat;
        }

        // [-1; 1]
        float NextSignedFloat()
        {
            return 2.f * NextFloat() - 1.f;
        }

        float Range(float from, float to)
		{
			return from + (to - from) * NextFloat();
		}

		float Range(Math::Vector2 v)
		{
			return v.x + (v.y - v.x) * NextFloat();
		}

        uint64_t operator()()
        {
            return NextUInt();
        }
    };

    using PerlinNoiseGenerator = siv::BasicPerlinNoise<float, RndGenerator, uint64_t>;    

    // All coordinates are in pixels
    using MaybeCastRes = eastl::optional<PhysScene::RaycastDesc>;
    MaybeCastRes RayCast(Math::Vector3 from, Math::Vector3 dir, float length, uint32_t group);
    MaybeCastRes RayCastLine(Math::Vector3 from, Math::Vector3 to, uint32_t group);
    bool RayHit(Math::Vector3 from, Math::Vector3 dir, float length, uint32_t group);
    bool RayHitLine(Math::Vector3 from, Math::Vector3 to, uint32_t group);

    MaybeCastRes SphereCast(Math::Vector3 from, Math::Vector3 dir, float length, float radius, uint32_t group);
    MaybeCastRes SphereCastLine(Math::Vector3 from, Math::Vector3 to, float radius, uint32_t group);
    bool SphereHit(Math::Vector3 from, Math::Vector3 dir, float length, float radius, uint32_t group);
    bool SphereHitLine(Math::Vector3 from, Math::Vector3 to, float radius, uint32_t group);

    MaybeCastRes RayCastBox(Math::Vector3 from, Math::Vector3 dir, float length, Math::Matrix boxTrans, Math::Vector3 boxHalfSize);

    void DebugArrow(Math::Vector3 from, Math::Vector3 to, Color color);
    void DebugTrinagle(Math::Vector3 pos, Utils::Angle from, Utils::Angle to, float length, Color color);
    void DebugSector(Math::Vector3 pos, Utils::Angle from, Utils::Angle to, Color color, float step = 3.f, float length = 50.f);

    Math::Vector2 GetCursorPos(int aliasX, int aliasY);

    void AddCameraShake(float addTrauma);

    void FormatTimer(float timer, eastl::string& result);

    struct AutoReloadTechnique
    {
        RenderTechniqueRef tech;

        uint64_t lastWriteTime = 0;

        bool TryReload(const char *shader);
        bool TryReload();

        RenderTechniqueRef operator->()
        {
            return tech;
        }

        operator RenderTechniqueRef()
        {
            return tech;
        }

        void operator=(RenderTechniqueRef _tech)
        {
            tech = _tech;
        }
    };

    float GetRandom(float from, float to);

    float GetRandom(Math::Vector2 v);

    Utils::Angle AdjustAngleByDirection(Utils::Angle angle, float direction);

    Math::Matrix MakeMatrix(Math::Vector3 up, Math::Vector3 pos);

    template<typename T, typename U>
    inline bool IsFlipped(T dir, U up)
    {
        return Utils::Vector::xy(dir).Cross(Utils::Vector::xy(up)) < -0.01f;
    }

    inline eastl::tuple<bool, bool, bool> NormalToFlags(Math::Vector3 normal)
	{
		const bool onWall    = Math::IsNonZero(normal.x);
		const bool onCeiling = Math::IsNonZero(normal.y) && normal.y < 0.f;
		const bool onGround  = Math::IsNonZero(normal.y) && normal.y > 0.f;
		return eastl::make_tuple(onWall, onCeiling, onGround);
	}

    eastl::optional<Math::Vector3> GetMovingSurfaceVel(Utils::MaybeCastRes castRes);

    void ConvertScene(Scene* scene);
    void ConvertAllScenes();

    void PlaySoundEvent(const char* name, Math::Vector3 *pos = nullptr);
}